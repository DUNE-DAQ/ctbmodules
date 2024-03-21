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
       s.field("hlt_0_count", self.uint8, 0, doc="Random hardware trigger."),
       s.field("hlt_1_count", self.uint8, 0, doc="Beam particle trigger."),
       s.field("hlt_2_count", self.uint8, 0, doc="Beam particle trigger with Cherenkov based particle selection. HP1 and LP1. (1-2GeV electrons)"),
       s.field("hlt_3_count", self.uint8, 0, doc="Beam particle trigger with Cherenkov based particle selection. HP0 and LP1. (3-5GeV pions)"),
       s.field("hlt_4_count", self.uint8, 0, doc="Beam particle trigger with Cherenkov based particle selection. HP1 and LP0."),
       s.field("hlt_5_count", self.uint8, 0, doc="CRT trigger - any US and DS coincidence within 240 ns. Only enabled outside the beam spill."),
       s.field("hlt_6_count", self.uint8, 0, doc="CRT trigger - any US and DS coincidence within 240 ns. Onbeam and Offbeam"),
       s.field("hlt_7_count", self.uint8, 0, doc="Beam particle trigger with Cherenkov based particle selection. Excludes HP1 and LP1. (1-2GeV hadrons)"),
       s.field("hlt_8_count", self.uint8, 0, doc="CRT trigger - Jura-side US and DS "),
       s.field("hlt_9_count", self.uint8, 0, doc="CRT trigger - Saleve-side US and DS"),
       s.field("hlt_10_count", self.uint8, 0, doc="CRT trigger - Jura-side US and Saleve-side DS"),
       s.field("hlt_11_count", self.uint8, 0, doc="Spare"),
       s.field("hlt_12_count", self.uint8, 0, doc="Spare"),
       s.field("hlt_13_count", self.uint8, 0, doc="Spare"),
       s.field("hlt_14_count", self.uint8, 0, doc="Spare"),
       s.field("llt_0_count", self.uint8, 0, doc="Random LLT"),
       s.field("llt_1_count", self.uint8, 0, doc="Beam particle trigger."),
       s.field("llt_2_count", self.uint8, 0, doc="High Pressure Cherenkov (C713)"),
       s.field("llt_3_count", self.uint8, 0, doc="Low Pressure Cherenkov (C716)"),
       s.field("llt_4_count", self.uint8, 0, doc="Spare"),
       s.field("llt_5_count", self.uint8, 0, doc="Spare"),
       s.field("llt_6_count", self.uint8, 0, doc="Beam gate"),
       s.field("llt_7_count", self.uint8, 0, doc="Spare"),
       s.field("llt_8_count", self.uint8, 0, doc="Spare"),
       s.field("llt_9_count", self.uint8, 0, doc="Spare"),
       s.field("llt_10_count", self.uint8, 0, doc="CRT pixels around beam pipe."),
       s.field("llt_11_count", self.uint8, 0, doc="Spare"),
       s.field("llt_12_count", self.uint8, 0, doc="Any CRT pixel on US Saleve side"),
       s.field("llt_13_count", self.uint8, 0, doc="Any CRT pixel on DS Saleve side"),
       s.field("llt_14_count", self.uint8, 0, doc="Spare"),
       s.field("llt_15_count", self.uint8, 0, doc="Any upstream CRT pixel"),
       s.field("llt_16_count", self.uint8, 0, doc="Any downstream CRT pixel"),
       s.field("llt_17_count", self.uint8, 0, doc="Spare"),
       s.field("llt_18_count", self.uint8, 0, doc="Any CRT on US Jura side"),
       s.field("llt_19_count", self.uint8, 0, doc="Any CRT on DS Jura side")
   ], doc="Central Trigger Board Module Information")
};

moo.oschema.sort_select(info) 
