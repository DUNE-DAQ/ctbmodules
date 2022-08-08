// The schema used by classes in the appfwk code tests.
//
// It is an example of the lowest layer schema below that of the "cmd"
// and "app" and which defines the final command object structure as
// consumed by instances of specific DAQModule implementations (ie,
// the test/Pacman* modules).

local moo = import "moo.jsonnet";

// A schema builder in the given path (namespace)
local ns = "dunedaq.ctbmodules.ctbmodule";
local s = moo.oschema.schema(ns);

// Object structure used by the test/fake producer module
local ctbmodule = {

    boolean : s.boolean("boolean",
                      doc="A boolean"),

    uint8 : s.number("uint8", "u8",
                      doc="An 8 byte unsigned field"),

    string : s.string("String",
                       doc="A string field"),

    conf: s.record("Conf", [

        s.field("buffer_size", self.uint8, 5000,
                doc="CTB Word Buffer Size"),

        s.field("receiver_connection_timeout", self.uint8, 10,
                doc="CTB Receiver Connection Timeout value (microseconds)"),

        s.field("control_connection_port", self.uint8, 8991,
                doc="CTB Control Connection Port"),

        s.field("receiver_connection_port", self.uint8, 0,
                doc="CTB Receiver Connection Port"),

        s.field("ctb_hostname", self.string, "np04-ctb-1",
                doc="CTB Hostname"),

        s.field("routing_table_config", self.string, "@local::routing_table_config_BRs",
                doc="CTB Routing table config"),

        s.field("request_mode", self.string, "Window", //Could also be "Buffer" mode
                doc="CTB Request Mode"),

        s.field("request_port", self.uint8, 3001,
                doc="CTB Request Port"),

        s.field("request_address", self.string, "227.128.12.26",
                doc="CTB Multicast Request Address"),

        s.field("request_window_width", self.uint8, 500000,
                doc="CTB Request Window Width"), // 3 s of window width. This is an ideally infinite window mode

        s.field("request_window_offset", self.uint8, 375000,
                doc="CTB Request Window Offset"), // Request message contains tzero. Window will be from tzero - offset to tz-o + width

        s.field("request_window_are_unique", self.boolean, true,
                doc="CTB Request Window Uniqueness"), 

        s.field("separate_data_thread", self.boolean, true,
                doc="CTB Data Thread Separation"), 

        s.field("circular_buffer_mode", self.boolean, true,
                doc="CTB Circular Buffer Mode"), 

        s.field("data_buffer_depth_fragments", self.uint8, 12000,
                doc="CTB Data Buffer Depth (num fragments)"), // CTB Default 1000

        s.field("board_address", self.string, "np04-ctb-1",
                doc="CTB Board Address"),

        s.field("control_port", self.uint8, 8991,
                doc="CTB Control Port"),

        s.field("group_size", self.uint8, 1,
                doc="CTB Group Size"), // this sets how many pakages from the board should be grouped to form a fragment. Default 1

        s.field("word_buffer_size", self.uint8, 10000,
                doc="CTB Word Buffer Size"), // default 5000

        s.field("metric_TS_interval", self.uint8, 50,
                doc="CTB Metric TS Interval"), 

        s.field("throw_exception", self.boolean, true,
                doc="CTB Throw Exception"), 

        s.field("calibration_stream_output", self.string, "/scratch/ctb_calib",
                doc="CTB Calibration Stream Output Path"),

        s.field("calibration_update", self.uint8, "5",
                doc="CTB Calibration Update Interval"),

        s.field("run_trigger_output", self.string, "/nfs/sw/trigger/counters",
                doc="CTB Trigger Output Path"),
 
        s.field("receiver_timeout_scaling", self.uint8, 4,
                doc="CTB Receiver Timeout Scaling"), // default 2

        s.field("nADCcounts", self.uint8, 100,
                doc="CTB nADC Counts"), 

        s.field("throttle_usecs", self.uint8, 100,
                doc="CTB Throttle Time (microseconds)"),  // Wait this many usecs before creating the next event

        s.field("distribution_type", self.uint8, 1,
                doc="CTB Distribution Type"), 

        s.field("fragment_ids", self.string, "[0]",
                doc="CTB Fragment IDs"), // In the case of just one fragment, "fragment_id: 0" would also work

        s.field("board_id", self.uint8, 999,
                doc="CTB Board ID"), 

        s.field("board_config", self.string, "", doc="CTB Configuration"),

    ], doc="Central Trigger Board DAQ Module Configuration"),

};

// WPV: Metrics - do we need to port these?
//metrics: {
//dim: {
//metricPluginType: dim 
//level: 5 
//reporting_interval: 5.0 
//Verbose: false 
//DNSPort: 2505 
//DNSNode: "np04-srv-024.cern.ch" 
//DIMServerName: TriggerBoardReader 
//IDName: trigger_0 
//}
//}

moo.oschema.sort_select(ctbmodule, ns)
