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
        s.field("standalone_enable", self.boolean, false),
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
        s.field("trigger", self.hlt_trigger_seq, [
            {"id":"HLT_1", "description":"Reconstructable track beam trigger, no PDS selection, no CRT selection", "enable":false, "minc":"0x2", "mexc":"0x0", "prescale":"0x1"},
            {"id":"HLT_2", "description":"Reconstructable track beam trigger, particle selection with Cherenkov detectors (HP=1,LP=1), no PDS selection, no CRT selection", "enable":false, "minc":"0xE", "mexc":"0x0", "prescale":"0x1"},
            {"id":"HLT_3", "description":"Cherenkov particle selection with with C713=0 & C716=1, no CRT, no PDS", "enable":false, "minc":"0xA", "mexc":"0x4", "prescale":"0x1"},
            {"id":"HLT_4", "description":"Spare", "enable":false, "minc":"0x0", "mexc":"0x0", "prescale":"0x1"},
            {"id":"HLT_5", "description":"CRT trigger US/DS off-spill with a prescale of 3", "enable":false, "minc":"0x18000", "mexc":"0x40", "prescale":"0x3"},
            {"id":"HLT_6", "description":"Test CRT trigger US/DS without beam consideration with no prescale", "enable":false, "minc":"0x18000", "mexc":"0x0", "prescale":"0x1"},
            {"id":"HLT_7", "description":"Low beam energy hadron trigger (no electrons), Cherenkov selection (C1=0,C2=0), no PDS, no CRT", "enable":false, "minc":"0x2", "mexc":"0xC", "prescale":"0x1"},
            {"id":"HLT_8", "description":"Crossing muons Jura side", "enable":false, "minc":"0xC0000", "mexc":"0x0", "prescale":"0x1"},
            {"id":"HLT_9", "description":"Crossing muons Saleve side", "enable":false, "minc":"0x3000", "mexc":"0x0", "prescale":"0x1"},
            {"id":"HLT_10", "description":"HV current limit threshold", "enable":false, "minc":"0x80", "mexc":"0x0", "prescale":"0x1"},
            {"id":"HLT_11", "description":"Ground plane signals", "enable":false, "minc":"0x100", "mexc":"0x0", "prescale":"0x1"},
            {"id":"HLT_12", "description":"Purity monitor signals", "enable":false, "minc":"0x200", "mexc":"0x0", "prescale":"0x1"},
            {"id":"HLT_13", "description":"Spare", "enable":false, "minc":"0x2", "mexc":"0x0", "prescale":"0x1"},
            {"id":"HLT_14", "description":"Spare", "enable":false, "minc":"0x2", "mexc":"0x0", "prescale":"0x1"},
            {"id":"HLT_15", "description":"Spare", "enable":false, "minc":"0x2", "mexc":"0x0", "prescale":"0x1"},
            {"id":"HLT_16", "description":"Spare", "enable":false, "minc":"0x2", "mexc":"0x0", "prescale":"0x1"},
            {"id":"HLT_17", "description":"Spare", "enable":false, "minc":"0x2", "mexc":"0x0", "prescale":"0x1"}
        ]),
    ], doc="Central Trigger Board HLT Configuration"),

    llt_count_trigger: s.record("Llt_count_trigger",  [
        s.field("id", self.string, ""),
        s.field("description", self.string, ""),
        s.field("enable", self.boolean, false),
        s.field("mask", self.string, ""),
        s.field("type", self.string, ""),
        s.field("count", self.string, "0x1"),
    ], doc="Central Trigger Board HLT Command LLT Trigger Configuration"),

    llt_count_trigger_seq: s.sequence("Llt_count_trigger_seq", self.llt_count_trigger, doc="Central Trigger Board HLT Trigger Sequence Configuration"),

    llt_mask_trigger: s.record("Llt_mask_trigger",  [
        s.field("id", self.string, ""),
        s.field("description", self.string, ""),
        s.field("enable", self.boolean, false),
        s.field("mask", self.string, ""),
    ], doc="Central Trigger Board HLT Command LLT Trigger Reduced Configuration"),

    llt_mask_trigger_seq: s.sequence("Llt_mask_trigger_seq", self.llt_mask_trigger, doc="Central Trigger Board LLT Trigger Sequence Configuration"),

    pds: s.record("Pds",  [
        s.field("channel_mask", self.string, "0xFFFFFF"),
        s.field("reshape_length", self.uint8, 5),
        s.field("delays", self.array, [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]),
        s.field("dac_thresholds", self.array, [2185,2194,2170,2176,2167,2200,2179,2197,2188,2179,2185,2191,2176,2182,2191,2182,2176,2173,2200,2185,2182,2176,2191,2167]),
        s.field("triggers", self.llt_count_trigger_seq, [
            {"id":"LLT_14", "description":"PDS at least 3 channels (on Jura side)", "enable":false, "mask":"0xF", "type":"0x1", "count":"0x2"},
            {"id":"LLT_17", "description":"PDS include 2 SSPs near the beam plug", "enable":false, "mask":"0x3", "type":"0x2", "count":"0x2"},
            {"id":"LLT_22", "description":"PDS at least 10 SSPs firing overall. The calibration input is masked out", "enable":false, "mask":"0xFFDFFF", "type":"0x1", "count":"0x9"},
            {"id":"LLT_23", "description":"Spare", "enable":false, "mask":"0x3", "type":"0x2", "count":"0x2"},
            {"id":"LLT_24", "description":"Spare", "enable":false, "mask":"0x3", "type":"0x2", "count":"0x2"}
        ]),
    ], doc="Central Trigger Board PDF Subsystem Configuration"),

    crt: s.record("Crt",  [
        s.field("channel_mask", self.string, "0xFFFFFFFF"),
        s.field("pixelate", self.boolean, true),
        s.field("reshape_length", self.uint8, 5),
        s.field("delays", self.array, [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]),
        s.field("triggers", self.llt_count_trigger_seq, [
            {"id":"LLT_11", "description":"Select CRT pixels around beam pipe (upstream)", "enable":false, "mask":"0xF000", "type":"0x1", "count":"0x0"},
            {"id":"LLT_12", "description":"Any upstream CRT Saleve side", "enable":false, "mask":"0x60FC", "type":"0x1", "count":"0x0"},
            {"id":"LLT_13", "description":"Any downstream CRT Saleve side", "enable":false, "mask":"0x9F030000", "type":"0x1", "count":"0x0"},
            {"id":"LLT_15", "description":"Any upstream CRT", "enable":false, "mask":"0x60FC", "type":"0x1", "count":"0x0"},
            {"id":"LLT_16", "description":"Any downstream CRT", "enable":false, "mask":"0xFFFF0000", "type":"0x1", "count":"0x0"},
            {"id":"LLT_18", "description":"Spare", "enable":false, "mask":"0x0", "type":"0x1", "count":"0x0"},
            {"id":"LLT_19", "description":"Spare", "enable":false, "mask":"0x0", "type":"0x1", "count":"0x0"},
            {"id":"LLT_20", "description":"Spare", "enable":false, "mask":"0x0", "type":"0x1", "count":"0x0"},
            {"id":"LLT_21", "description":"Spare", "enable":false, "mask":"0x0", "type":"0x1", "count":"0x0"}
        ]),
    ], doc="Central Trigger Board CRT Subsystem Configuration"),

    beam: s.record("Beam",  [
        s.field("channel_mask", self.string, "0x1FB"),
        s.field("reshape_length", self.uint8, 50),
        s.field("delays", self.array, [1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0]),
        s.field("triggers", self.llt_mask_trigger_seq, [
            {"id":"LLT_1", "description":"Beam trigger", "enable":false, "mask":"0x1E3"},
            {"id":"LLT_2", "description":"Mask in High Pressure Cherenkov (C713)", "enable":false, "mask":"0x8"},
            {"id":"LLT_3", "description":"Mask in Low Pressure Cherenkov (C716)", "enable":false, "mask":"0x10"},
            {"id":"LLT_4", "description":"Spare", "enable":false, "mask":"0x0"},
            {"id":"LLT_5", "description":"Spare", "enable":false, "mask":"0x0"},
            {"id":"LLT_6", "description":"Beam gate selection", "enable":false, "mask":"0x2"},
            {"id":"LLT_7", "description":"HV current limit threshold (init stream)", "enable":false, "mask":"0x200"},
            {"id":"LLT_8", "description":"Ground plane signals", "enable":false, "mask":"0x400"},
            {"id":"LLT_9", "description":"Purity monitor", "enable":false, "mask":"0x1000"},
            {"id":"LLT_10", "description":"Spare", "enable":false, "mask":"0x0"},
            {"id":"LLT_25", "description":"Spare", "enable":false, "mask":"0x0"},
            {"id":"LLT_26", "description":"Spare", "enable":false, "mask":"0x0"}
        ]),
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

        s.field("hsievent_connection_name", self.string, 
                doc="Connection name to be used to send hsievent to"),

        s.field("receiver_connection_timeout", self.uint8, 1000,
                doc="CTB Receiver Connection Timeout value (microseconds)"),

        s.field("control_connection_port", self.uint8, 8991,
                doc="CTB Control Connection Port"),

        s.field("ctb_hostname", self.string, "np04-ctb-1",
                doc="CTB Hostname"),

        s.field("calibration_stream_output", self.string, "",
                doc="CTB Calibration Stream Output Path"),

        s.field("calibration_update", self.uint8, "5",
                doc="CTB Calibration Update Interval"),

        s.field("run_trigger_output", self.string, "/nfs/sw/trigger/counters",
                doc="CTB Trigger Output Path"),
 
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
