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
#include "rcif/cmd/Nljs.hpp"

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
#define CTB_HSI_FRAME_VERSION 0x1
#define CTB_HSI_DET_ID 0x1
#define CTB_HSI_CRATE_ID 0x0
#define CTB_HSI_SLOT_ID 0x0

namespace dunedaq {
namespace ctbmodules {

CTBModule::CTBModule(const std::string& name)
  : hsilibs::HSIEventSender(name)
  , m_is_running(false)
  , m_stop_requested(false)
  , m_is_configured(false)
  , m_error_state(false)
  , m_total_hlt_counter(0)
  , m_ts_word_counter(0) 
  , m_hlt_trigger_counter()
  , m_llt_trigger_counter()
  , m_control_ios()
  , m_receiver_ios()
  , m_control_socket(m_control_ios)
  , m_receiver_socket(m_receiver_ios)
  , m_thread_(std::bind(&CTBModule::do_hsi_work, this, std::placeholders::_1))
  , m_has_calibration_stream( false )
  , m_run_HLT_counter(0)
  , m_run_LLT_counter(0)
  , m_run_channel_status_counter(0)
  , m_num_control_messages_sent(0)
  , m_num_control_responses_received(0)
  , m_last_readout_hlt_timestamp(0)
{
  register_command("conf", &CTBModule::do_configure);
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
CTBModule::init(const nlohmann::json& init_data)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering init() method";
  HSIEventSender::init(init_data);
  m_llt_hsi_data_sender = get_iom_sender<dunedaq::hsilibs::HSI_FRAME_STRUCT>(appfwk::connection_uid(init_data, "llt_output"));
  m_hlt_hsi_data_sender = get_iom_sender<dunedaq::hsilibs::HSI_FRAME_STRUCT>(appfwk::connection_uid(init_data, "hlt_output"));

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting init() method";
}

void
CTBModule::do_configure(const data_t& args)
{

  TLOG_DEBUG(0) << get_name() << ": Configuring CTB";

  m_cfg = args.get<ctbmodule::Conf>();
  m_receiver_port = m_cfg.board_config.ctb.sockets.receiver.port;  
  m_timeout = std::chrono::microseconds( m_cfg.receiver_connection_timeout ) ;

  TLOG_DEBUG(0) << get_name() << ": Board receiver network location " << m_cfg.board_config.ctb.sockets.receiver.host << ':' << m_cfg.board_config.ctb.sockets.receiver.port << std::endl;

  // Initialise monitoring variables
  m_num_control_messages_sent = 0;
  m_num_control_responses_received = 0;
  m_ts_word_counter = 0;

  std::map<std::string, size_t> id_to_idx;
  for(size_t i = 0; i < m_hlt_range; i++) id_to_idx["HLT_" + std::to_string(i)] = i;
  for(size_t i = 0; i < m_llt_range; i++) id_to_idx["LLT_" + std::to_string(i)] = i;

  nlohmann::json random_triggers = m_cfg.board_config.ctb.misc;

  // HLTs
  // 0th HLT is random trigger that's not in HLT array
  if (random_triggers["randomtrigger_1"]["enable"]) m_hlt_trigger_counter[0] = 0;
  nlohmann::json trigger_array = m_cfg.board_config.ctb.HLT.trigger;
  for (const auto& trigger : trigger_array) { if (trigger["enable"]) m_hlt_trigger_counter[id_to_idx[trigger["id"]]] = 0; }

  // LLTs: Beam and CRT
  // 0th LLT is random trigger that's not in HLT array
  if (random_triggers["randomtrigger_2"]["enable"]) m_llt_trigger_counter[0] = 0;
  trigger_array = m_cfg.board_config.ctb.subsystems.crt.triggers;
  for (const auto& trigger : trigger_array) { if (trigger["enable"]) m_llt_trigger_counter[id_to_idx[trigger["id"]]] = 0; }

  trigger_array = m_cfg.board_config.ctb.subsystems.beam.triggers;
  for (const auto& trigger : trigger_array) { if (trigger["enable"]) m_llt_trigger_counter[id_to_idx[trigger["id"]]] = 0; }

  // network connection to ctb hardware control
  boost::asio::ip::tcp::resolver resolver( m_control_ios ); 
  boost::asio::ip::tcp::resolver::query query(m_cfg.ctb_hostname, std::to_string(m_cfg.control_connection_port) ) ; //"np04-ctb-1", 8991
  boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query) ;

  m_endpoint = iter->endpoint(); 
 
  // TODO should we put this into a try?
  m_control_socket.connect( m_endpoint );

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

  // create the json string
  nlohmann::json config;
  to_json(config, m_cfg.board_config);
  //TLOG() << "CONF TEST: " << config.dump();

  send_config(config.dump());
}

void
CTBModule::do_start(const nlohmann::json& startobj)
{

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_start() method";

  // Set this to false early so it doesn't interfere with the start
  m_stop_requested.store(false);

  auto start_params = startobj.get<rcif::cmd::StartParams>();
  m_run_number.store(start_params.run);

  m_total_hlt_counter.store(0);

  TLOG_DEBUG(0) << get_name() << ": Sending start of run command";
  m_thread_.start_working_thread();

  if ( m_has_calibration_stream ) {
    std::stringstream run;
    run << "run" << start_params.run;
    SetCalibrationStream(run.str()) ;
  }

  if ( send_message( "{\"command\":\"StartRun\"}" )  ) {
    m_is_running.store(true);
    TLOG_DEBUG(1) << get_name() << ": successfully started";
  }
  else{
    throw CTBCommunicationError(ERS_HERE, "Unable to start CTB");
  }

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_start() method";
}

void
CTBModule::do_stop(const nlohmann::json& /*stopobj*/)
{

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_stop() method";

  TLOG_DEBUG(0) << get_name() << ": Sending stop run command" << std::endl;
    
  // Give the do_work thread a chance to stop before stopping the CTB,
  // otherwise we end up reading from an empty buffer
  m_stop_requested.store(true);
  std::this_thread::sleep_for(std::chrono::milliseconds(2));

  if(send_message( "{\"command\":\"StopRun\"}" ) ){
    TLOG_DEBUG(1) << get_name() << ": successfully stopped";
    m_is_running.store( false ) ;
  }
  else{
    throw CTBCommunicationError(ERS_HERE, "Unable to stop CTB");
  }
  store_run_trigger_counters( m_run_number ) ; 
  m_thread_.stop_working_thread();

  m_run_HLT_counter=0;
  m_run_LLT_counter=0;
  m_run_channel_status_counter=0;

  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Exiting do_stop() method";
}

void
CTBModule::do_hsi_work(std::atomic<bool>& running_flag)
{
  TLOG_DEBUG(TLVL_ENTER_EXIT_METHODS) << get_name() << ": Entering do_work() method";

  std::size_t n_bytes = 0 ;
  std::size_t n_words = 0 ;

  const size_t header_size = sizeof( content::tcp_header_t ) ;
  const size_t word_size = content::word::word_t::size_bytes ;

  TLOG_DEBUG(TLVL_CTB_MODULE) << get_name() <<  ": Header size: " << header_size << std::endl << "Word size: " << word_size << std::endl;

  //connect to socket
  boost::asio::ip::tcp::acceptor acceptor(m_receiver_ios, boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), m_receiver_port ) );
  TLOG_DEBUG(0) << get_name() << ": Waiting for an incoming connection on port " << m_receiver_port << std::endl;

  std::future<void> accepting = async( std::launch::async, [&]{ acceptor.accept(m_receiver_socket) ; } ) ;

  while ( running_flag.load() && !m_stop_requested.load() ) {
    if ( accepting.wait_for( m_timeout ) == std::future_status::ready ){
      break ;
    }
  }

  TLOG_DEBUG(0) << get_name() <<  ": Connection received: start reading" << std::endl;

  content::tcp_header_t head ;
  head.packet_size = 0;
  content::word::word_t temp_word ;
  boost::system::error_code receiving_error;
  bool connection_closed = false ;
  uint64_t ch_stat_beam, ch_stat_crt, ch_stat_pds;
  uint64_t prev_timestamp = 0;
  ts_payload prev_hlt, prev_llt, prev_ch_stat;
  ts_payload curr_hlt, curr_llt, curr_ch_stat;
  // buffers for word matching. buf_a are the trigger words, buf_b are corresponding payloads
  std::queue<content::word::trigger_t> match_buf_a_hlts, match_buf_a_llts;
  std::queue<ts_payload> match_buf_b_llts, match_buf_b_chstatus;

  while (running_flag.load() && !m_stop_requested.load()) {

    update_calibration_file();

    if ( ! read( head ) ) {
      connection_closed = true ;
      break;
    }

    n_bytes = head.packet_size ;
    // extract n_words

    n_words = n_bytes / word_size ;
    // read n words as requested from the header
    
    update_buffer_counts(n_words);

    for ( unsigned int i = 0 ; i < n_words ; ++i ) {
      
      if (!running_flag.load() || m_stop_requested.load()) {
        break;
      }

      //read a word
      if ( ! read( temp_word ) ) {
        connection_closed = true ;
        break ;
      }
      // put it in the calibration stream
      if ( m_has_calibration_stream ) {
        m_calibration_file.write( reinterpret_cast<const char*>( & temp_word ), word_size ) ;
        m_calibration_file.flush() ;
      }          // word printing in calibration stream
      
      //check if it is a TS word and increment the counter
      if ( IsTSWord( temp_word ) ) {
        ++m_ts_word_counter;
        TLOG_DEBUG(9) << "Received timestamp word! TS: "+temp_word.timestamp;
        prev_timestamp = temp_word.timestamp;
      }

      else if ( IsFeedbackWord( temp_word ) ) {
        m_error_state.store( true ) ;
        content::word::feedback_t * feedback = reinterpret_cast<content::word::feedback_t*>( & temp_word ) ;
        TLOG_DEBUG(7) << "Received feedback word!";

        TLOG_DEBUG(8) << get_name() << ": Feedback word: " << std::endl
                                                  << std::hex 
                                                  << " \t Type -> " << feedback -> word_type << std::endl 
                                                  << " \t TS -> " << feedback -> timestamp << std::endl
                                                  << " \t Code -> " << feedback -> code << std::endl
                                                  << " \t Source -> " << feedback -> source << std::endl
                                                  << " \t Padding -> " << feedback -> padding << std::dec << std::endl ;
      } else if (temp_word.word_type == content::word::t_gt)
      {
        TLOG_DEBUG(3) << "Received HLT word! TS: " + temp_word.timestamp;
        content::word::trigger_t * hlt_word = reinterpret_cast<content::word::trigger_t*>( & temp_word );
        curr_hlt = {hlt_word->timestamp, (hlt_word->trigger_word & 0x1FFFFFFFFFFFFFFF)};
        if (check_repeated_word(curr_hlt, prev_hlt, temp_word.word_type)) continue;
        match_buf_a_hlts.push(*hlt_word);
        // Count the total HLTs and each specific one
        ++m_run_HLT_counter;
        ++m_total_hlt_counter;
        for (auto &hlt : m_hlt_trigger_counter) { if( (hlt_word->trigger_word >> hlt.first) & 0x1 ) ++hlt.second; }
        m_last_readout_hlt_timestamp = temp_word.timestamp;
        prev_hlt = curr_hlt;
      }
      else if (temp_word.word_type == content::word::t_lt)
      {
        TLOG_DEBUG(5) << "Received LLT word! TS: " + temp_word.timestamp;
        content::word::trigger_t * llt_word = reinterpret_cast<content::word::trigger_t*>( & temp_word ) ;
        curr_llt = {llt_word->timestamp, (llt_word->trigger_word & 0xFFFFFFFF)};
        if (check_repeated_word(curr_llt, prev_llt, temp_word.word_type)) continue;
        match_buf_a_llts.push(*llt_word);
        match_buf_b_llts.push(curr_llt);

        ++m_run_LLT_counter;
        for (auto &llt : m_llt_trigger_counter) { if( (llt_word->trigger_word >> llt.first) & 0x1 ) ++llt.second; }
        prev_llt = curr_llt;
      }
      else if (temp_word.word_type == content::word::t_ch)
      {

        content::word::ch_status_t * ch_stat_word = reinterpret_cast<content::word::ch_status_t*>( & temp_word ) ;
        // The channel status only has 60b TS so complete the upper 4b from the TS Word. (fyi 60b rolls over >500yr @ 62.5MHz) 
        uint64_t corrected_ts = ((prev_timestamp & 0xF000000000000000) | ch_stat_word->timestamp);
        TLOG_DEBUG(6) << "Received Channel Status word! TS: " + corrected_ts;
        ch_stat_beam = ch_stat_word->get_beam();
        ch_stat_crt  = ch_stat_word->get_crt();
        ch_stat_pds  = ch_stat_word->get_pds();
        curr_ch_stat = {
            corrected_ts, 
            ((ch_stat_pds << 48) | (ch_stat_crt << 16) | ch_stat_beam)
        };
        if (check_repeated_word(curr_ch_stat, prev_ch_stat, temp_word.word_type)) continue;
        match_buf_b_chstatus.push(curr_ch_stat);
        prev_ch_stat = curr_ch_stat;

        ++m_run_channel_status_counter;
      }
      // do matching
      match_between_buffers(match_buf_a_hlts, match_buf_b_llts, prev_hlt.first);
      match_between_buffers(match_buf_a_llts, match_buf_b_chstatus, prev_llt.first);

    } // n_words loop

    if ( connection_closed ){
      break ;
    }
  }

  // Make sure CTB run stops before closing socket
  while ( m_is_running.load() ) {
    std::this_thread::sleep_for(std::chrono::microseconds(100));
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

bool CTBModule::check_repeated_word(ts_payload& curr_word, ts_payload& prev_word, uint64_t wtype){
  if (curr_word.first == prev_word.first) { // words with repeated timestamp. Not good!
    std::stringstream msg;
    msg << "Multiple words have the same timestamp, Using the first one. Word type: ";
    if (wtype == content::word::word_type::t_gt) msg << "HLT";
    else if (wtype == content::word::word_type::t_lt) msg << "LLT";
    else if (wtype == content::word::word_type::t_ch) msg << "Channel Status";
    else msg << wtype;
    msg << ", TS: "<< curr_word.first << ".";
    if (curr_word.second != prev_word.second) { // not only do we have repeated timestamp, they have different payload...
      msg << " Different payload!! Previous payload: " << std::hex << prev_word.second
          << " Current payload: " << curr_word.second;
      ers::warning(CTBRepeatedTimestampWarning(ERS_HERE, msg.str()));
    } else {
      msg << "Both have payload " << std::hex << prev_word.second;
      ers::info(CTBRepeatedTimestampWarning(ERS_HERE, msg.str()));
    }
    return true;
  }
  return false;
}

void CTBModule::send_matched_trigger_word(content::word::trigger_t& word, uint64_t payload) {
  // Send HSI data to a DLH
  std::array<uint32_t, 7> hsi_struct;
  bool is_hlt = word.IsHLT();
  hsi_struct[0] = (is_hlt << 26)            |  // link
                  (CTB_HSI_SLOT_ID << 22)   |  
                  (CTB_HSI_CRATE_ID << 12)  | 
                  (CTB_HSI_DET_ID << 6)     |
                  CTB_HSI_FRAME_VERSION
                  ;
  hsi_struct[1] = word.timestamp;        // ts low
  hsi_struct[2] = word.timestamp >> 32;  // ts high
  hsi_struct[3] = payload;                // lower 32b
  hsi_struct[4] = payload >> 32;          // upper 32b (will be 0x0 for llt payloads)
  hsi_struct[5] = word.trigger_word;     // trigger_map;
  hsi_struct[6] = is_hlt ? m_run_HLT_counter : m_run_LLT_counter; // m_generated_counter;
  int dbg_lvl = is_hlt ? 4 : 6;
  TLOG_DEBUG(dbg_lvl) << get_name() << ": Formed HSI_FRAME_STRUCT for " << (is_hlt? "HLT" : "LLT")
      << std::hex 
      << "0x"   << hsi_struct[0]
      << ", 0x" << hsi_struct[1]
      << ", 0x" << hsi_struct[2]
      << ", 0x" << hsi_struct[3]
      << ", 0x" << hsi_struct[4]
      << ", 0x" << hsi_struct[5]
      << ", 0x" << hsi_struct[6]
      << "\n";
  if (is_hlt) {
    send_raw_hsi_data(hsi_struct, m_hlt_hsi_data_sender.get());
    // TODO properly fill device id
    dfmessages::HSIEvent event(0x1, word.trigger_word, word.timestamp, m_run_HLT_counter, m_run_number);
    send_hsi_event(event);
  }
  else {
    send_raw_hsi_data(hsi_struct, m_llt_hsi_data_sender.get());
  }
  
}

void CTBModule::match_between_buffers(std::queue<content::word::trigger_t>& buf_a, std::queue<ts_payload>& buf_b, uint64_t timeout_reference) {
  while (buf_a.size() > 0) {
    content::word::trigger_t trigger = buf_a.front();
    bool is_hlt = trigger.IsHLT();
    uint64_t trigger_ts = trigger.timestamp;
    uint64_t trigger_word = trigger.trigger_word;
    if (timeout_reference > (trigger_ts + 100)) {
      std::stringstream msg;
      msg << "Time out while waiting for a match for the "<< (is_hlt? "HLT" : "LLT")
          << ": TS = " << trigger_ts << ", trigger word = " << trigger_word
          << " Timeout reference: " << timeout_reference;
      ers::warning(CTBWordMatchWarning(ERS_HERE, msg.str()));
      buf_a.pop();
      continue;
    }
    if (buf_b.size() == 0) break; // no input to match yet. Return for now, wait for matching input to come
    while (buf_b.size() > 0) {
      uint64_t input_ts = buf_b.front().first;
      if (input_ts < (trigger_ts - 1)) { // word is too early. No longer needed
        buf_b.pop();
      }
      else if (input_ts == (trigger_ts - 1)) { // match is found
        send_matched_trigger_word(trigger, buf_b.front().second);
        buf_a.pop();
        buf_b.pop();
        break;
      }
      else { // buf_b is already past the match window. No matching is found, error!
        if (is_hlt && (trigger_word == 0x1 || trigger_word == (0x1 << 16))) { // Fake HLTs, no matching is OK
          send_matched_trigger_word(trigger, 0);
        } 
        else{
          std::stringstream msg;
          msg << "No match found for " << (is_hlt? "HLT" : "LLT")
              << ": TS = " << trigger_ts << ", trigger word = " << trigger_word;
          ers::warning(CTBWordMatchWarning(ERS_HERE, msg.str()));
        }
        buf_a.pop();
        break;
      }
    } // end loop buf_b
  } // end loop buf_a
  // Don't let buf_b get too long (e.g. when LLT rate is high but HLT rate is low)
  while (buf_b.size() > 32) buf_b.pop();
}


template<typename T>
bool CTBModule::read( T &obj) {

  boost::system::error_code receiving_error;
  boost::asio::read( m_receiver_socket, boost::asio::buffer( &obj, sizeof(T) ), receiving_error ) ;

  if ( ! receiving_error ) {
    return true ;
  }

  if ( receiving_error == boost::asio::error::eof) {
    std::string error_message = "Socket closed: " + receiving_error.message();
    ers::error(CTBCommunicationError(ERS_HERE, error_message));
    return false ;
  }

  if ( receiving_error ) {
    std::string error_message = "Read failure: " + receiving_error.message();
    ers::error(CTBCommunicationError(ERS_HERE, error_message));
    return false ;
  }

  return true ;
}

bool CTBModule::IsTSWord( const content::word::word_t &w ) noexcept {

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
  TLOG_DEBUG(0) << get_name() << ": New Calibration Stream file: " << global_name << std::endl ;

}

void CTBModule::update_calibration_file() {

  if ( ! m_has_calibration_stream ) {
    return ;
  }
  
  std::chrono::steady_clock::time_point check_point = std::chrono::steady_clock::now();
  
  if ( check_point - m_last_calibration_file_update < m_calibration_file_interval ) {
    return ;
  }

  m_calibration_file.close() ;
  init_calibration_file() ;

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

    TLOG_DEBUG(1) << get_name() << ": Resetting before configuring" << std::endl;
    send_reset();

  }

  TLOG_DEBUG(1) << get_name() << ": Sending config" << std::endl;

  if ( send_message( config ) ) {

    m_is_configured.store(true) ;

  }
  else{
      throw CTBCommunicationError(ERS_HERE, "Unable to configure CTB");
  }
}

void CTBModule::send_reset() {

  TLOG_DEBUG(1) << get_name() << ": Sending a reset" << std::endl;

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
  TLOG_DEBUG(1) << get_name() << ": Sending message: " << msg;

  m_num_control_messages_sent++;

  boost::asio::write( m_control_socket, boost::asio::buffer( msg ), error ) ;
  boost::array<char, 1024> reply_buf{" "} ;
  m_control_socket.read_some( boost::asio::buffer(reply_buf ), error);
  std::stringstream raw_answer( std::string(reply_buf .begin(), reply_buf .end() ) ) ;
  TLOG_DEBUG(1) << get_name() << ": Unformatted answer: " << raw_answer.str(); 

  nlohmann::json answer ;
  raw_answer >> answer ;
  nlohmann::json & messages = answer["feedback"] ;
  TLOG_DEBUG(1) << get_name() << ": Received messages: " << messages.size();

  bool ret = true ;
  for (nlohmann::json::size_type i = 0; i != messages.size(); ++i ) {
    
    m_num_control_responses_received++;

    std::string type = messages[i]["type"].dump() ;
    if ( type.find("error") != std::string::npos || type.find("Error") != std::string::npos || type.find("ERROR") != std::string::npos ) {
      ers::error(CTBMessage(ERS_HERE, messages[i]["message"].dump()));
      ret = false ;
    }
    else if ( type.find("warning") != std::string::npos || type.find("Warning") != std::string::npos || type.find("WARNING") != std::string::npos ) {
      ers::warning(CTBMessage(ERS_HERE, messages[i]["message"].dump()));
    }
    else if ( type.find("info") != std::string::npos || type.find("Info") != std::string::npos || type.find("INFO") != std::string::npos) {
      TLOG() << "Message from the board: " << messages[i]["message"].dump();
    }
    else {
      std::stringstream blob;
      blob << messages[i] ;
      TLOG() << get_name() << ": Unformatted from the board: " << blob.str();
    }
  }

  return ret;
  
}

void
CTBModule::update_buffer_counts(uint new_count) // NOLINT(build/unsigned)
{
  std::unique_lock mon_data_lock(m_buffer_counts_mutex);
  if (m_buffer_counts.size() > 1000)
    m_buffer_counts.pop_front();
  m_buffer_counts.push_back(new_count);
}

double
CTBModule::read_average_buffer_counts()
{
  std::unique_lock mon_data_lock(m_buffer_counts_mutex);

  double total_counts;
  uint32_t number_of_counts; // NOLINT(build/unsigned)

  total_counts = 0;
  number_of_counts = m_buffer_counts.size();

  if (number_of_counts) {
    for (uint i = 0; i < number_of_counts; ++i) { // NOLINT(build/unsigned)
      total_counts = total_counts + m_buffer_counts.at(i);
    }
    return total_counts / number_of_counts;
  } else {
    return 0;
  }
}

void CTBModule::get_info(opmonlib::InfoCollector& ci, int /*level*/)
{
  dunedaq::ctbmodules::ctbmoduleinfo::CTBModuleInfo module_info;

  module_info.num_control_messages_sent = m_num_control_messages_sent.load();
  module_info.num_control_responses_received = m_num_control_responses_received.load();
  module_info.ctb_hardware_run_status = m_is_running; 
  module_info.ctb_hardware_configuration_status = m_is_configured;
    
  module_info.last_readout_timestamp = m_last_readout_hlt_timestamp.load();
  module_info.failed_to_send_hsi_events_counter = m_failed_to_send_counter.load();
  module_info.last_sent_timestamp = m_last_sent_timestamp.load();
  module_info.average_buffer_occupancy = read_average_buffer_counts();

  module_info.total_hlt_count = m_total_hlt_counter.load();
  module_info.ts_word_count = m_ts_word_counter.exchange(0);

  for (auto &hlt : m_hlt_trigger_counter) {
    opmonlib::InfoCollector tmp_ic;
    dunedaq::ctbmodules::ctbmoduleinfo::LevelTriggerInfo ti;
    ti.count = hlt.second.exchange(0);
    tmp_ic.add(ti);
    ci.add("hlt_" + std::to_string(hlt.first), tmp_ic);
  }

  for (auto &llt : m_llt_trigger_counter) {
    opmonlib::InfoCollector tmp_ic;
    dunedaq::ctbmodules::ctbmoduleinfo::LevelTriggerInfo ti;
    ti.count = llt.second.exchange(0);
    tmp_ic.add(ti);
    ci.add("llt_" + std::to_string(llt.first), tmp_ic);
  }

  ci.add(module_info);
}

} // namespace ctbmodules
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::ctbmodules::CTBModule)

// Local Variables:
// c-basic-offset: 2
// End:
