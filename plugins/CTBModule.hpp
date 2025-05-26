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

#ifndef CTBMODULES_PLUGINS_CTBMODULE_HPP_
#define CTBMODULES_PLUGINS_CTBMODULE_HPP_

#include "appfwk/DAQModule.hpp"
#include "appmodel/CTBModule.hpp"
#include "iomanager/Receiver.hpp"
#include "iomanager/Sender.hpp"
#include "utilities/WorkerThread.hpp"

#include "hsilibs/HSIEventSender.hpp"

#include <ers/Issue.hpp>

#include "ctbmodules/opmon/CTBModule.pb.h"

#include "CTBPacketContent.hpp"

#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <shared_mutex>
#include <map>
#include <deque>

#include <boost/asio.hpp>
#include <boost/array.hpp>


namespace dunedaq {
namespace ctbmodules {

  typedef std::pair<uint64_t,uint64_t> ts_payload;  // NOLINT

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

  void init(std::shared_ptr<appfwk::ConfigurationManager> cfgMgr) override;

  static bool IsTSWord( const content::word::word_t &w ) noexcept;
  static bool IsFeedbackWord( const content::word::word_t &w ) noexcept;
  bool ErrorState() const { return m_error_state.load() ; } 

protected:
  void generate_opmon_data() override;
  
private:

  // control and monitoring variables

  std::atomic<bool> m_is_running;
  std::atomic<bool> m_stop_requested;
  std::atomic<bool> m_is_configured;

  unsigned int m_receiver_port;
  std::chrono::microseconds m_timeout;
  std::atomic<bool> m_error_state;

  boost::asio::io_service m_control_ios;
  boost::asio::io_service m_receiver_ios;
  boost::asio::ip::tcp::socket m_control_socket;
  boost::asio::ip::tcp::socket m_receiver_socket;
  boost::asio::ip::tcp::endpoint m_endpoint;

  std::shared_ptr<dunedaq::hsilibs::HSIEventSender::raw_sender_ct> m_llt_hsi_data_sender;
  std::shared_ptr<dunedaq::hsilibs::HSIEventSender::raw_sender_ct> m_hlt_hsi_data_sender;

  ts_payload last_popped_llt, last_popped_chstatus;


  // Commands
  void do_configure(const nlohmann::json& obj) override;
  void do_start(const nlohmann::json& startobj) override;
  void do_stop(const nlohmann::json& obj) override;
  void do_scrap(const nlohmann::json& /*obj*/) override{};

  void send_reset() ;
  void send_config(const std::string & config);
  bool send_message(const std::string & msg);

  // Configuration
  std::shared_ptr<appfwk::ConfigurationManager> m_cfg;
  using conf_t = appmodel::CTBModule;
  const conf_t* m_module = nullptr;
  
  std::atomic<daqdataformats::run_number_t> m_run_number;

  // Threading
  dunedaq::utilities::WorkerThread m_thread_;
  void do_hsi_work(std::atomic<bool>&);
  // variables for geo_id to construct the HSI frame
  // These are defined as uint32 in the schema, but given the way the HSI frame is consctructed from this it is unsusable.
  // THe HSI frame uses 4 Bits for the slot and 10 Bits for the crate and 6 for the DetID. So here I'm overiding the types
  uint16_t m_det;  // NOLINT
  uint16_t m_crate;  // NOLINT
  uint16_t m_slot;   // NOLINT

  // Generate HSI Frame/Event
  void send_matched_trigger_word(const content::word::trigger_t&, uint64_t);  // NOLINT
  void match_between_buffers(std::queue<content::word::trigger_t>&, std::queue<ts_payload>&, uint64_t, content::word::word_type);  // NOLINT

  static bool check_repeated_word(ts_payload&, ts_payload&, uint64_t);  // NOLINT
  
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

  // metric utilities
  using general_metric_t = dunedaq::ctbmodules::opmon::CTBModuleInfo;
  using channel_metric_t = dunedaq::ctbmodules::opmon::TriggerInfo;

  using const_total_hlt_counter_t = std::invoke_result<decltype(&general_metric_t::total_hlt_count),
						       general_metric_t>::type;
  std::atomic<std::remove_const<const_total_hlt_counter_t>::type> m_total_hlt_counter;
  
  using const_ts_word_counter_t = std::invoke_result<decltype(&general_metric_t::ts_word_count),
						     general_metric_t>::type;
  std::atomic<std::remove_const<const_ts_word_counter_t>::type> m_ts_word_counter;

  size_t m_hlt_range = 20;
  size_t m_llt_range = 25;
  using const_trigger_counter_t = std::invoke_result<decltype(&channel_metric_t::count),
						     channel_metric_t>::type;
  using trigger_counter_t = std::remove_const<const_ts_word_counter_t>::type;
  std::map<size_t, std::atomic<trigger_counter_t>> m_hlt_trigger_counter;
  std::map<size_t, std::atomic<trigger_counter_t>> m_llt_trigger_counter;

  std::atomic<trigger_counter_t> m_run_HLT_counter = 0;
  std::atomic<trigger_counter_t> m_run_LLT_counter = 0;
  std::atomic<trigger_counter_t> m_run_channel_status_counter = 0;

  // monitoring
  std::deque<uint> m_buffer_counts; // NOLINT(build/unsigned)
  std::shared_mutex m_buffer_counts_mutex;
  void update_buffer_counts(uint new_count); // NOLINT(build/unsigned)
  double read_average_buffer_counts();

  using const_message_counter_t = std::invoke_result<decltype(&general_metric_t::num_control_messages_sent),
                                                     general_metric_t>::type;
  using message_counter_t =  std::remove_const<const_message_counter_t>::type;
  std::atomic<message_counter_t> m_num_control_messages_sent = 0;
  std::atomic<message_counter_t> m_num_control_responses_received = 0;
  std::atomic<uint64_t> m_last_readout_hlt_timestamp = 0; // NOLINT(build/unsigned)

};
} // namespace ctbmodules
} // namespace dunedaq

#endif // CTBMODULES_PLUGINS_CTBMODULE_HPP_ 

// Local Variables:
// c-basic-offset: 2
// End:
