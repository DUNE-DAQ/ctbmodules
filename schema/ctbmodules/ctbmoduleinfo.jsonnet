// This is the application info schema used by the data link handler module.
// It describes the information object structure passed by the application 
// for operational monitoring

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.ctbmodules.ctbmoduleinfo");

local info = {
    uint8  : s.number("uint8", "u8", doc="An unsigned of 8 bytes"),
    float8 : s.number("float8", "f8", doc="A float of 8 bytes"),
    choice : s.boolean("Choice"),
    string : s.string("String", moo.re.ident, doc="A string field"),
    double_val: s.number("DoubleValue", "f8", doc="A double"),

   info: s.record("CTBModuleInfo", [
       s.field("num_control_messages_sent", self.uint8, 0, doc="Number of control messages sent to CTB"),
       s.field("num_control_responses_received", self.uint8, 0, doc="Number of control message responses received from CTB"),
       s.field("ctb_hardware_run_status", self.choice, 0, doc="Run status of CTB hardware itself"),
       s.field("ctb_hardware_configuration_status", self.choice, 0, doc="Configuration status of CTB hardware itself"),
       s.field("sent_hsi_events_counter", self.uint8, 0, doc="Number of sent HSIEvents so far"), 
       s.field("failed_to_send_hsi_events_counter", self.uint8, 0, doc="Number of failed send attempts so far"),
       s.field("last_sent_timestamp", self.uint8, 0, doc="Timestamp of the last sent HSIEvent"),
       s.field("last_readout_timestamp", self.uint8, 0, doc="Timestamp of the last read HLT word"),
       s.field("average_buffer_occupancy", self.double_val, 0, doc="Average (word) occupancy of buffer in CTB firmware."),
       s.field("total_hlt_count", self.uint8, 0, doc="Total HLT count for a run."),
       s.field("ts_word_count", self.uint8, 0, doc="Timestamp word count. Fixed frequency heartbeat."),
   ], doc="Central Trigger Board Module Information"),

   trigger: s.record("LevelTriggerInfo", [
       s.field("count", self.uint8, 0, doc="Count for a single level trigger"),
   ], doc="Level Trigger information")

};

moo.oschema.sort_select(info) 
