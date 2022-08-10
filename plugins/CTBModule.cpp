/**
 * @file CTBModule.cpp CTBModule class
 * implementation
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "CTBModule.hpp"
#include "CTBModuleIssues.hpp"

#include "appfwk/DAQModuleHelper.hpp"
#include "iomanager/IOManager.hpp"
#include "logging/Logging.hpp"

#include <chrono>
#include <string>
#include <thread>
#include <vector>

/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "CTBModule" // NOLINT
#define TLVL_ENTER_EXIT_METHODS 10
#define TLVL_CTB_MODULE 15

namespace dunedaq {
namespace ctbmodules {

CTBModule::CTBModule(const std::string& name)
  : DAQModule(name)
  , m_is_running(false)
  , m_is_configured(false)
  , m_n_TS_words(0)
  , m_error_state(false)
  , m_control_ios()
  , m_receiver_ios()
  , m_control_socket(m_control_ios)
  , m_receiver_socket(m_receiver_ios)
  , thread_(std::bind(&CTBModule::do_work, this, std::placeholders::_1))
  , m_has_calibration_stream( false )
{
  register_command("start", &CTBModule::do_start);
  register_command("stop", &CTBModule::do_stop);
}

CTBModule::~CTBModule(){
    //check if running. and in case stop the run
  if(m_is_running){
    const nlohmann::json stopobj;
    do_stop(stopobj);
  } 
  m_control_socket.close() ;

}

void
CTBModule::init(const nlohmann::json& /*iniobj*/)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering init() method";

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting init() method";
}

void
CTBModule::do_configure(const data_t& args)
{
  m_cfg = args.get<ctbmodule::Conf>();
  m_receiver_port = m_cfg.receiver_connection_port;  
  m_buffer_size = m_cfg.buffer_size; //5000

  // Initialise monitoring variables
  m_num_control_messages_sent = 0;
  m_num_control_responses_received = 0;

  boost::asio::ip::tcp::resolver resolver( m_control_ios ); 
  boost::asio::ip::tcp::resolver::query query(m_cfg.ctb_hostname, std::to_string(m_cfg.control_connection_port) ) ; //"np04-ctb-1", 8991
  boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query) ;

  m_endpoint = iter->endpoint(); 
 
  //shoudl we put this into a try?
  m_control_socket.connect( m_endpoint );

  // prepare the receiver
  // get the json configuration string
  //std::stringstream json_stream(static_cast<nlohmann::json>(m_cfg.board_config)) ;
  //nlohmann::json jblob;
  //json_stream >> jblob;

  //nlohmann::json conf_json = nlohmann::json::parse(m_cfg.board_config);

  // get the receiver port from the json
  const unsigned int receiver_port = m_cfg.board_config.ctb.sockets.receiver.port;
  m_rollover = m_cfg.board_config.ctb.sockets.receiver.rollover;
  const unsigned int timeout_scaling = m_cfg.receiver_timeout_scaling;
  const unsigned int timeout = m_rollover / 50 / timeout_scaling; //microseconds

  //                                      |-> this is the board clock frequency which is 50 MHz
  m_timeout = std::chrono::microseconds( timeout ) ;
  // if necessary, set the calibration stream

  if ( m_cfg.calibration_stream_output != "")  {
    m_has_calibration_stream = true ; 
    m_calibration_dir = m_cfg.calibration_stream_output ;
    m_calibration_file_interval = std::chrono::minutes(m_cfg.calibration_update); 
  }

  if ( m_cfg.run_trigger_output != "" ) {
    m_has_run_trigger_report = true ; 
    m_run_trigger_dir = m_cfg.run_trigger_output;
    if ( m_run_trigger_dir.back() != '/' ) m_run_trigger_dir += '/' ;
  }

  // complete the json configuration
  // with the receiver host which is the machines where the board reader is running

  const std::string receiver_address = boost::asio::ip::host_name() ;
  m_cfg.board_config.ctb.sockets.receiver.host = receiver_address ;
  TLOG() << get_name() << "Board packages receved at " << receiver_address << ':' << receiver_port << std::endl;

  // create the json string
  nlohmann::json config;
  to_json(config, m_cfg.board_config);
 
  send_config(config.dump());

}

void
CTBModule::do_start(const nlohmann::json& /*startobj*/)
{

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_start() method";

  TLOG() << get_name() << "Sending start of run command";

  if ( m_has_calibration_stream ) {
    std::stringstream run;
    run << "run" << 91919191;//run_number();
    SetCalibrationStream(run.str()) ;
  }



  if ( send_message( "{\"command\":\"StartRun\"}" )  ) {
    m_is_running.store(true);
    TLOG() << get_name() << " successfully started";
  }
  else{
    ers::error(CTBCommunicationError(ERS_HERE, "Unable to start CTB"));
  }

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_start() method";
}

void
CTBModule::do_stop(const nlohmann::json& /*stopobj*/)
{

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_stop() method";

  TLOG() << get_name() << "Sending stop run command" << std::endl;

  if(send_message( "{\"command\":\"StopRun\"}" ) ){
    TLOG() << get_name() << " successfully stopped";

    m_is_running.store( false ) ;
  }
  else{
    ers::error(CTBCommunicationError(ERS_HERE, "Unable to stop CTB"));
  }
  store_run_trigger_counters( 91919191 ) ; 

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_stop() method";
}

void
CTBModule::do_work(std::atomic<bool>& running_flag)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_work() method";

  std::size_t n_bytes = 0 ;
  std::size_t n_words = 0 ;

  const size_t header_size = sizeof( content::tcp_header_t ) ;
  const size_t word_size = content::word::word_t::size_bytes ;

  // the raw buffer can contain 4 times the maximum TCP package size, which is 4 KB
  boost::lockfree::spsc_queue< content::word::word_t > word_buffer(m_buffer_size);

  TLOG() << get_name() <<  "Header size: " << header_size << std::endl << "Word size: " << word_size << std::endl;

  //connect to socket
  boost::asio::ip::tcp::acceptor acceptor(m_receiver_ios, boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), m_receiver_port ) );
  TLOG() << get_name() << "Waiting for an incoming connection on port " << m_receiver_port << std::endl;

  std::future<void> accepting = async( std::launch::async, [&]{ acceptor.accept(m_receiver_socket) ; } ) ;

  while ( running_flag.load() ) {
    if ( accepting.wait_for( m_timeout ) == std::future_status::ready ){
      break ;
    }
  }

  TLOG() << get_name() <<  "Connection received: start reading" << std::endl;

  content::tcp_header_t head ;
  head.packet_size = 0;
  content::word::word_t temp_word ;
  boost::system::error_code receiving_error;
  bool connection_closed = false ;

  while (running_flag.load()) {

    update_calibration_file();

    if ( ! read( head ) ) {
      connection_closed = true ;
      break;
    }

    n_bytes = head.packet_size ;
    //    dune::DAQLogger::LogInfo("CTB_Receiver") << "Package size "  << n_bytes << std::endl ;
    // extract n_words

    n_words = n_bytes / word_size ;
    // read n words as requested from the header

    for ( unsigned int i = 0 ; i < n_words ; ++i ) {
      //read a word
      if ( ! read( temp_word ) ) {
        connection_closed = true ;
        break ;
      }

      // put it in the calibration stream
      if ( m_has_calibration_stream ) {
        m_calibration_file.write( reinterpret_cast<const char*>( & temp_word ), word_size ) ;
        m_calibration_file.flush() ;
        //dune::DAQLogger::LogInfo("CTB_Receiver") << "Word Type: " << temp_word.frame.word_type << std::endl ;
      }          // word printing in calibration stream
      
      //check if it is a TS word and increment the counter
      if ( IsTSWord( temp_word ) ) {
        m_n_TS_words++ ;
      }

      else if ( IsFeedbackWord( temp_word ) ) {
        m_error_state.store( true ) ;
        content::word::feedback_t * feedback = reinterpret_cast<content::word::feedback_t*>( & temp_word ) ;
        TLOG() << get_name() << "Feedback word: " << std::endl
                                                  << std::hex 
                                                  << " \t Type -> " << feedback -> word_type << std::endl 
                                                  << " \t TS -> " << feedback -> timestamp << std::endl
                                                  << " \t Code -> " << feedback -> code << std::endl
                                                  << " \t Source -> " << feedback -> source << std::endl
                                                  << " \t Padding -> " << feedback -> padding << std::dec << std::endl ;
      }

      // push the word
      while ( ! word_buffer.push( temp_word )) {
        ers::warning(CTBBufferWarning(ERS_HERE, "Word Buffer full and cannot store more data")) ;
      }

    } // n_words loop

    if ( connection_closed ){
      break ;
    }
  }

  boost::system::error_code closing_error;

  if ( m_error_state.load() ) {

    m_receiver_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, closing_error);

    if ( closing_error ) {
      std::stringstream msg;
      msg << "Error in shutdown " << closing_error.message();
      ers::error(CTBCommunicationError(ERS_HERE,msg.str())) ;
    }

  }

  m_receiver_socket.close(closing_error) ;

  if ( closing_error ) {
    std::stringstream msg;
    msg << "Socket closing failed:: " << closing_error.message();
    ers::error(CTBCommunicationError(ERS_HERE,msg.str()));
  }


  TLOG_DEBUG(TLVL_CTB_MODULE) << get_name() << ": End of do_work loop: stop receiving data from the CTB";
  
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_work() method";

}


template<typename T>
bool CTBModule::read( T &obj) {

  boost::system::error_code receiving_error;
  boost::asio::read( m_receiver_socket, boost::asio::buffer( &obj, sizeof(T) ), receiving_error ) ;

  if ( ! receiving_error ) {
    return true ;
  }

  if ( receiving_error == boost::asio::error::eof) {
    TLOG() << get_name() <<  "Socket closed: "<< receiving_error.message()  << std::endl ;
    return false ;
  }

  

  if ( receiving_error ) {
    TLOG() << get_name() << "Read failure: " << receiving_error.message() << std::endl ;
    return false ;
  }

  return true ;
}

bool CTBModule::IsTSWord( const content::word::word_t &w ) noexcept {

  //dune::DAQLogger::LogInfo("CTB_Receiver") << "word type " <<  w.frame.word_type  << std::endl ;

  if ( w.word_type == content::word::t_ts ) {
    return true;
  }
  return false;

}

bool CTBModule::IsFeedbackWord( const content::word::word_t &w ) noexcept {

  if ( w.word_type == content::word::t_fback ) {
    return true;
  }
  return false;

}

void CTBModule::init_calibration_file() {

  if ( ! m_has_calibration_stream ){
    return ;
  } 
  char file_name[200] = "" ;
  time_t rawtime;
  time( & rawtime ) ;
  struct tm * timeinfo = localtime( & rawtime ) ;
  strftime( file_name, sizeof(file_name), "%F_%H.%M.%S.calib", timeinfo );
  std::string global_name = m_calibration_dir + m_calibration_prefix + file_name ;
  m_calibration_file.open( global_name, std::ofstream::binary ) ;
  m_last_calibration_file_update = std::chrono::steady_clock::now();
  // _calibration_file.setf ( std::ios::hex, std::ios::basefield );
  // _calibration_file.unsetf ( std::ios::showbase );
  TLOG() << get_name() << "New Calibration Stream file: " << global_name << std::endl ;

}

bool CTBModule::SetCalibrationStream( const std::string & prefix ) {

  if ( m_calibration_dir.back() != '/' ){
    m_calibration_dir += '/' ;
  }
  m_calibration_prefix = prefix ; 
  if ( prefix.size() > 0 ){ 
    m_calibration_prefix += '_' ;
  } 
  // possibly we could check here if the directory is valid and  writable before assuming the calibration stream is valid
  return true ;

}

bool CTBModule::store_run_trigger_counters( unsigned int run_number, const std::string & prefix) const {

  if ( ! m_has_run_trigger_report ) {
    return false ;
  }

  std::stringstream out_name ;
  out_name << m_run_trigger_dir << prefix << "run_" << run_number << "_triggers.txt";
  std::ofstream out( out_name.str() ) ;
  out << "Good Part\t " << m_run_gool_part_counter << std::endl 
      << "Total HLT\t " << m_run_HLT_counter << std::endl ;

  for ( unsigned int i = 0; i < m_metric_HLT_names.size() ; ++i ) {
    out << "HLT " << i << " \t " << m_run_HLT_counters[i] << std::endl ;
  }

  return true; 

}


void CTBModule::send_config( const std::string & config ) {

  if ( m_is_configured.load() ) {

    TLOG() << get_name() << "Resetting before configuring" << std::endl;
    send_reset();

  }

  TLOG() << get_name() << "Sending config" << std::endl;

  if ( send_message( config ) ) {

    m_is_configured.store(true) ;

  }
  else{
      ers::error(CTBCommunicationError(ERS_HERE, "Unable to configure CTB"));
  }
}

void CTBModule::send_reset() {

  TLOG() << get_name() << "Sending a reset" << std::endl;

  if(send_message( "{\"command\":\"HardReset\"}" )){

    m_is_running.store(false);
    m_is_configured.store(false);

  }
  else{
    ers::error(CTBCommunicationError(ERS_HERE, "Unable to reset CTB"));
  }

}

bool CTBModule::send_message( const std::string & msg ) {

  //add error options                                                                                                

  boost::system::error_code error;
  TLOG() << get_name() << "Sending message: " << msg;

  m_num_control_messages_sent++;

  boost::asio::write( m_control_socket, boost::asio::buffer( msg ), error ) ;
  boost::array<char, 1024> reply_buf{" "} ;
  m_control_socket.read_some( boost::asio::buffer(reply_buf ), error);
  std::stringstream raw_answer( std::string(reply_buf .begin(), reply_buf .end() ) ) ;
  TLOG() << get_name() << "Unformatted answer: " << raw_answer.str(); 

  nlohmann::json answer ;
  raw_answer >> answer ;
  nlohmann::json & messages = answer["feedback"] ;
  TLOG() << get_name() << "Received messages: " << messages.size();

  bool ret = true ;
  for (nlohmann::json::size_type i = 0; i != messages.size(); ++i ) {
    
    m_num_control_responses_received++;

    std::string type = messages[i]["type"].dump() ;
    if ( type.find("error") != std::string::npos || type.find("Error") != std::string::npos || type.find("ERROR") != std::string::npos ) {
      TLOG() << get_name() << "Error from the Board: " << messages[i]["message"].dump();
      ret = false ;
    }
    else if ( type.find("warning") != std::string::npos || type.find("Warning") != std::string::npos || type.find("WARNING") != std::string::npos ) {
      TLOG() << get_name() << "Warning from the Board: " << messages[i]["message"].dump();
    }
    else if ( type.find("info") != std::string::npos || type.find("Info") != std::string::npos || type.find("INFO") != std::string::npos) {
      TLOG() << get_name() << "Info from the board: " << messages[i]["message"].dump();
    }
    else {
      std::stringstream blob;
      blob << messages[i] ;
      TLOG() << get_name() << "Unformatted from the board: " << blob.str();
    }
  }

  return ret;
  
}

void CTBModule::get_info(opmonlib::InfoCollector& ci, int /*level*/){

    dunedaq::ctbmodules::ctbmoduleinfo::CTBModuleInfo moduleInfo;

    moduleInfo.num_control_messages_sent = 
    moduleInfo.num_control_responses_received = 
    moduleInfo.ctb_hardware_run_status = m_is_running; 
    moduleInfo.ctb_hardware_configuration_status = m_is_configured;
    moduleInfo.num_ts_words_received = m_n_TS_words;
    
    ci.add(moduleInfo);
  }





} // namespace ctbmodules
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::ctbmodules::CTBModule)

// Local Variables:
// c-basic-offset: 2
// End:
