/**
 * @file CTBModule.hpp
 *
 * CTBModule is a DAQModule implementation that reads that provides a command and readout
 * interface to the Central Trigger Board hardware.
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef CTBMODULE_PLUGINS_CTBModule_HPP_
#define CTBMODULE_PLUGINS_CTBModule_HPP_

#include "appfwk/DAQModule.hpp"
#include "iomanager/Receiver.hpp"
#include "iomanager/Sender.hpp"
#include "utilities/WorkerThread.hpp"

#include <ers/Issue.hpp>

#include "CTBPacketContent.hpp"

#include "ctbmodules/ctbmodule/Nljs.hpp"
#include "ctbmodules/ctbmoduleinfo/InfoNljs.hpp"

#include <memory>
#include <string>
#include <vector>
#include <fstream>

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/lockfree/spsc_queue.hpp>

namespace dunedaq {
namespace ctbmodules {

/**
 * @brief CTBModule provides the command and readout interface to the Central Trigger Board hardware
 */
class CTBModule : public dunedaq::appfwk::DAQModule
{
public:
  /**
   * @brief CTBModule Constructor
   * @param name Instance name for this CTBModule instance
   */
  explicit CTBModule(const std::string& name);
  ~CTBModule();

  CTBModule(const CTBModule&) = delete;            ///< CTBModule is not copy-constructible
  CTBModule& operator=(const CTBModule&) = delete; ///< CTBModule is not copy-assignable
  CTBModule(CTBModule&&) = delete;                 ///< CTBModule is not move-constructible
  CTBModule& operator=(CTBModule&&) = delete;      ///< CTBModule is not move-assignable

  void init(const nlohmann::json& iniobj) override;

  bool SetCalibrationStream( const std::string &string_dir, 
                             const std::chrono::minutes &interval, 
                             const std::string &prefix = "" );

  static bool IsTSWord( const content::word::word_t &w ) noexcept;
  static bool IsFeedbackWord( const content::word::word_t &w ) noexcept;
  bool ErrorState() const { return m_error_state.load() ; } 

  void get_info(opmonlib::InfoCollector& ci, int level) override;
  
private:

  // control variables

  std::atomic<bool> m_is_running;
  std::atomic<bool> m_is_configured;

  /*const */unsigned int m_receiver_port;
  std::chrono::microseconds m_timeout;
  std::atomic<unsigned int> m_n_TS_words;
  std::atomic<bool> m_error_state;
  unsigned int m_buffer_size;

  boost::asio::io_service m_control_ios;
  boost::asio::io_service m_receiver_ios;
  boost::asio::ip::tcp::socket m_control_socket;
  boost::asio::ip::tcp::socket m_receiver_socket;
  boost::asio::ip::tcp::endpoint m_endpoint;

  // Commands
  void do_configure(const nlohmann::json& obj);
  void do_start(const nlohmann::json& obj);
  void do_stop(const nlohmann::json& obj);

  void send_reset() ;
  void send_config(const std::string & config);
  bool send_message(const std::string & msg);

  // Configuration
  dunedaq::ctbmodules::ctbmodule::Conf m_cfg;

  // Threading
  dunedaq::utilities::WorkerThread thread_;
  void do_work(std::atomic<bool>&);

  template<typename T>
  bool read(T &obj);

  // members related to calibration stream

  void update_calibration_file();
  void init_calibration_file();


  bool m_has_calibration_stream; 
  std::string m_calibration_dir; 
  std::string m_calibration_prefix; 
  std::chrono::minutes m_calibration_file_interval;  
  std::ofstream m_calibration_file;
  std::chrono::steady_clock::time_point m_last_calibration_file_update;

  // monitoring

  int m_num_control_messages_sent;
  int m_num_control_responses_received;

};
} // namespace ctbmodule
} // namespace dunedaq

#endif // CTBMODULE_PLUGINS_CTBModule_HPP_

// Local Variables:
// c-basic-offset: 2
// End:
