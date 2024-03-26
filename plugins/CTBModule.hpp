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

#include "hsilibs/HSIEventSender.hpp"

#include <ers/Issue.hpp>

#include "CTBPacketContent.hpp"

#include "ctbmodules/ctbmodule/Nljs.hpp"
#include "ctbmodules/ctbmoduleinfo/InfoNljs.hpp"

#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <shared_mutex>
#include <map>

#include <boost/asio.hpp>
#include <boost/array.hpp>

namespace dunedaq {
namespace ctbmodules {

/**
 * @brief CTBModule provides the command and readout interface to the Central Trigger Board hardware
 */
class CTBModule : public dunedaq::hsilibs::HSIEventSender
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

  static uint64_t MatchTriggerInput(const uint64_t trigger_ts, const std::pair<uint64_t,uint64_t> &prev_input, const std::pair<uint64_t,uint64_t> &prev_prev_input, bool hlt_matching) noexcept;
  static bool IsTSWord( const content::word::word_t &w ) noexcept;
  static bool IsFeedbackWord( const content::word::word_t &w ) noexcept;
  bool ErrorState() const { return m_error_state.load() ; } 

  void get_info(opmonlib::InfoCollector& ci, int level) override;
  
private:

  // control and monitoring variables

  std::atomic<bool> m_is_running;
  std::atomic<bool> m_stop_requested;
  std::atomic<bool> m_is_configured;

  /*const */unsigned int m_receiver_port;
  std::chrono::microseconds m_timeout;
  std::atomic<unsigned int> m_n_TS_words;
  std::atomic<bool> m_error_state;

  std::atomic<unsigned int> m_total_hlt_counter;
  std::atomic<unsigned int> m_ts_word_counter;

  size_t m_hlt_range = 20;
  size_t m_llt_range = 25;
  std::map<size_t, std::atomic<unsigned int>> m_hlt_trigger_counter;
  std::map<size_t, std::atomic<unsigned int>> m_llt_trigger_counter;

  boost::asio::io_service m_control_ios;
  boost::asio::io_service m_receiver_ios;
  boost::asio::ip::tcp::socket m_control_socket;
  boost::asio::ip::tcp::socket m_receiver_socket;
  boost::asio::ip::tcp::endpoint m_endpoint;

  std::shared_ptr<dunedaq::hsilibs::HSIEventSender::raw_sender_ct> m_llt_hsi_data_sender;
  std::shared_ptr<dunedaq::hsilibs::HSIEventSender::raw_sender_ct> m_hlt_hsi_data_sender;


  // Commands
  void do_configure(const nlohmann::json& obj);
  void do_start(const nlohmann::json& startobj);
  void do_stop(const nlohmann::json& obj);
  void do_scrap(const nlohmann::json& /*obj*/){}

  void send_reset() ;
  void send_config(const std::string & config);
  bool send_message(const std::string & msg);

  // Configuration
  dunedaq::ctbmodules::ctbmodule::Conf m_cfg;
  std::atomic<daqdataformats::run_number_t> m_run_number;

  // Threading
  dunedaq::utilities::WorkerThread m_thread_;
  void do_hsi_work(std::atomic<bool>&);

  template<typename T>
  bool read(T &obj);

  // members related to calibration stream

  void update_calibration_file();
  void init_calibration_file();
  bool SetCalibrationStream( const std::string &prefix = "" );

  bool m_has_calibration_stream = false; 
  std::string m_calibration_dir = ""; 
  std::string m_calibration_prefix = ""; 
  std::chrono::minutes m_calibration_file_interval;  
  std::ofstream m_calibration_file;
  std::chrono::steady_clock::time_point m_last_calibration_file_update;

  // members related to run trigger report

  bool m_has_run_trigger_report = false;
  std::string m_run_trigger_dir = "";
  bool store_run_trigger_counters( unsigned int run_number, const std::string & prefix = "" ) const;


  std::atomic<unsigned long> m_run_gool_part_counter = 0;
  std::atomic<unsigned long> m_run_HLT_counter = 0;
  // TODO should be atomic?
  unsigned long m_run_HLT_counters[8] = {0};
  std::atomic<unsigned long> m_run_LLT_counter;
  std::atomic<unsigned long> m_run_channel_status_counter = 0;
  // metric utilities

  const std::array<std::string, 8> m_metric_HLT_names  = { "CTB_HLT_0_rate",
                                                            "CTB_HLT_1_rate", 
                                                            "CTB_HLT_2_rate",
                                                            "CTB_HLT_3_rate",
                                                            "CTB_HLT_4_rate",
                                                            "CTB_HLT_5_rate",
                                                            "CTB_HLT_6_rate",
                                                            "CTB_HLT_7_rate" };


  // monitoring

  std::deque<uint> m_buffer_counts; // NOLINT(build/unsigned)
  std::shared_mutex m_buffer_counts_mutex;
  void update_buffer_counts(uint new_count); // NOLINT(build/unsigned)
  double read_average_buffer_counts();

  std::atomic<int> m_num_control_messages_sent;
  std::atomic<int> m_num_control_responses_received;
  std::atomic<uint64_t> m_last_readout_hlt_timestamp; // NOLINT(build/unsigned)

};
} // namespace ctbmodule
} // namespace dunedaq

#endif // CTBMODULE_PLUGINS_CTBModule_HPP_

// Local Variables:
// c-basic-offset: 2
// End:
