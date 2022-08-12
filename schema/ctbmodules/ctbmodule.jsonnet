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

    array: s.sequence("Array", self.uint8, doc="General Array Type"),

    receiver: s.record("Receiver",  [
        s.field("rollover", self.uint8, 125000),
        s.field("host", self.string, "localhost"),
        s.field("port", self.uint8, 8992),
     ], doc="Central Trigger Board Receiver Socket Configuration"),

    monitor: s.record("Monitor",  [
        s.field("enable", self.boolean, false),
        s.field("host", self.string, "localhost"),
        s.field("port", self.uint8, 8993),
     ], doc="Central Trigger Board Monitor Socket Configuration"),

    statistics: s.record("Statistics",  [
        s.field("enable", self.boolean, false),
        s.field("port", self.uint8, 8994),
        s.field("updt_period", self.uint8, 1),
     ], doc="Central Trigger Board Statistics Socket Configuration"),


    sockets: s.record("Sockets",  [
        s.field("receiver", self.receiver, self.receiver),
        s.field("monitor", self.monitor, self.monitor),
        s.field("statistics", self.statistics, self.statistics),
     ], doc="Central Trigger Board Sockets Configuration"),

    randomtrigger: s.record("Randomtrigger",  [
        s.field("description", self.string, "Random trigger that can optionally be set to fire only during beam spill"),
        s.field("enable", self.boolean, false),
        s.field("fixed_freq", self.boolean, true),
        s.field("beam_mode", self.boolean, true),
        s.field("period", self.uint8, 100000),
    ], doc="Central Trigger Board Random Trigger Configuration"),


    pulser: s.record("Pulser",  [
        s.field("enable", self.boolean, false),
        s.field("frequency", self.uint8, 50),
    ], doc="Central Trigger Board Pulser Configuration"),

    timing: s.record("Timing",  [
        s.field("address", self.string, "0xF0"),
        s.field("group", self.string, "0x0"),
        s.field("triggers", self.boolean, true),
        s.field("lockout", self.string, "0x10"),
    ], doc="Central Trigger Board Timing Configuration"),

    misc: s.record("Misc",  [
        s.field("randomtrigger_1", self.randomtrigger, self.randomtrigger),
        s.field("randomtrigger_2", self.randomtrigger, self.randomtrigger),
        s.field("pulser", self.pulser, self.pulser),
        s.field("timing", self.timing, self.timing),
        s.field("ch_status", self.boolean, false),
        s.field("standalone_enable", self.boolean, true),
    ], doc="Central Trigger Board Misc Configuration"),

    command_mask: s.record("Command_mask",  [
        s.field("description", self.string, "C=beam triggers, D=non-beam triggers, E=none, F=random trigger"),
        s.field("C", self.string, "0x8E"),
        s.field("D", self.string, "0x360"),
        s.field("E", self.string, "0x1C00"),
        s.field("F", self.string, "0x80000001"),
    ], doc="Central Trigger Board HLT Command Mask Configuration"),

    hlt_trigger: s.record("Hlt_trigger",  [
        s.field("id", self.string, ""),
        s.field("description", self.string, ""),
        s.field("enable", self.boolean, false),
        s.field("minc", self.string, ""),
        s.field("mexc", self.string, ""),
        s.field("prescale", self.string, "0x1"),
    ], doc="Central Trigger Board HLT Command HLT Trigger Configuration"),

    hlt_trigger_seq: s.sequence("Hlt_trigger_seq", self.hlt_trigger,  doc="Central Trigger Board HLT Trigger Sequence Configuration"),

    hlt: s.record("Hlt",  [
        s.field("command_mask", self.command_mask, self.command_mask),
        s.field("trigger", self.hlt_trigger_seq, [self.hlt_trigger]),
    ], doc="Central Trigger Board HLT Configuration"),

    llt_trigger: s.record("Llt_trigger",  [
        s.field("id", self.string, ""),
        s.field("description", self.string, ""),
        s.field("enable", self.boolean, false),
        s.field("mask", self.string, ""),
        s.field("type", self.string, ""),
        s.field("count", self.string, "0x1"),
    ], doc="Central Trigger Board HLT Command LLT Trigger Configuration"),

    llt_trigger_seq: s.sequence("Llt_trigger_seq", self.llt_trigger, doc="Central Trigger Board HLT Trigger Sequence Configuration"),

    llt_trigger_red: s.record("Llt_trigger_red",  [
        s.field("id", self.string, ""),
        s.field("description", self.string, ""),
        s.field("enable", self.boolean, false),
        s.field("mask", self.string, ""),
    ], doc="Central Trigger Board HLT Command LLT Trigger Reduced Configuration"),

    llt_trigger_red_seq: s.sequence("Llt_trigger_red_seq", self.llt_trigger_red, doc="Central Trigger Board LLT Trigger Sequence Configuration"),

    pds: s.record("Pds",  [
        s.field("channel_mask", self.string, "0xFFFFFF"),
        s.field("reshape_length", self.uint8, 5),
        s.field("delays", self.array, [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]),
        s.field("dac_thresholds", self.array, [2185,2194,2170,2176,2167,2200,2179,2197,2188,2179,2185,2191,2176,2182,2191,2182,2176,2173,2200,2185,2182,2176,2191,2167]),
        s.field("triggers", self.llt_trigger_seq, [self.llt_trigger]),
    ], doc="Central Trigger Board PDF Subsystem Configuration"),

    crt: s.record("Crt",  [
        s.field("channel_mask", self.string, "0xFFFFFFFF"),
        s.field("pixelate", self.boolean, true),
        s.field("reshape_length", self.uint8, 5),
        s.field("delays", self.array, [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]),
        s.field("triggers", self.llt_trigger_seq, [self.llt_trigger]),
    ], doc="Central Trigger Board CRT Subsystem Configuration"),

    beam: s.record("Beam",  [
        s.field("channel_mask", self.string, "0x17FB"),
        s.field("reshape_length", self.uint8, 50),
        s.field("delays", self.array, [1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0]),
        s.field("triggers", self.llt_trigger_red_seq, [self.llt_trigger]),
    ], doc="Central Trigger Board Beam Subsystem Configuration"),


    subsystems: s.record("Subsystems",  [
        s.field("pds", self.pds, self.pds),
        s.field("crt", self.crt, self.crt),
        s.field("beam", self.beam, self.beam),
        ], doc="Central Trigger Board Subsystem Configuration"),

    ctb: s.record("Ctb",  [
        s.field("sockets", self.sockets, self.sockets),
        s.field("misc", self.misc, self.misc),
        s.field("HLT", self.hlt, self.hlt),
        s.field("subsystems", self.subsystems, self.subsystems),
        ], doc="Central Trigger Board Configuration Object"),


    board_config: s.record("Board_config",  [
        s.field("ctb", self.ctb, self.ctb),
        ], doc="Central Trigger Board Configuration Wrapper"),


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

        s.field("fragment_ids", self.array, [0],
                doc="CTB Fragment IDs"), // In the case of just one fragment, "fragment_id: 0" would also work

        s.field("board_id", self.uint8, 999,
                doc="CTB Board ID"), 

        s.field("board_config", self.board_config, self.board_config, doc="CTB board config"),

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
