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

        s.field("board_config", self.string, '\\"ctb\\":{
                \"sockets\":{
                        \"receiver\":{
                        \"rollover\":125000,
                        \"host\":\"localhost\",
                        \"port\":8992
                },
                \"monitor\":{
                        \"enable\" : false,
                        \"host\": \"localhost\",
                        \"port\": 8993
                        },
                \"statistics\": {
                        \"enable\":false,
                        \"port\":8994,
                        \"updt_period\": 1
                }
                },
                \"misc\":{
                \"randomtrigger_1\":{
                        \"description\":\"Random trigger that can optionally be set to fire only during beam spill\",
                        \"enable\":false,
                        \"fixed_freq\":true,
                        \"beam_mode\":true,
                        \"period\":100000
                },
                \"randomtrigger_2\":{
                        \"description\":\"Random trigger that can optionally be set to fire only outside beam spill\",
                        \"enable\":false,
                        \"fixed_freq\":true,
                        \"beam_mode\":true,
                        \"period\":100000
                },
                \"pulser\":{
                        \"enable\":false,
                        \"frequency\":50
                },
                \"timing\":{
                        \"address\":\"0xF0\",
                        \"group\":\"0x0\",
                        \"triggers\":true,
                        \"lockout\":\"0x10\"
                },
                \"ch_status\":false,
                \"standalone_enable\": true
                },
                \"HLT\":{
                \"command_mask\" : {
                        \"description\": \"C=beam triggers, D=non-beam triggers, E=none, F=random trigger\",
                        \"C\" : \"0x8E\",
                        \"D\" : \"0x360\",
                        \"E\" : \"0x1C00\",
                        \"F\" : \"0x80000001\"
                },
                \"trigger\": [
                        { \"id\":\"HLT_1\",
                        \"description\": \"Reconstructable track beam trigger, no PDS selection, no CRT selection\",
                        \"enable\":false,
                        \"minc\" : \"0x2\",
                        \"mexc\" : \"0x0\",
                        \"prescale\" : \"0x1\"
                        },
                        { \"id\":\"HLT_2\",
                        \"description\": \"Reconstructable track beam trigger, particle selection with Cherenkov detectors (HP=1,LP=1), no PDS selection, no CRT selection\",
                        \"enable\":false,
                        \"minc\" : \"0xE\",
                        \"mexc\" : \"0x0\",
                        \"prescale\" : \"0x1\"
                        },
                        { \"id\":\"HLT_3\",
                        \"description\": \"Cherenkov particle selection with with C713=0 & C716=1, no CRT, no PDS\",
                        \"enable\":false,
                        \"minc\" : \"0xA\",
                        \"mexc\" : \"0x4\",
                        \"prescale\" : \"0x1\"
                        },
                        { \"id\":\"HLT_4\",
                        \"description\": \"Spare\",
                        \"enable\":false,
                        \"minc\" : \"0x0\",
                        \"mexc\" : \"0x0\",
                        \"prescale\" : \"0x1\"
                        },
                        { \"id\":\"HLT_5\",
                        \"description\": \"CRT trigger US/DS off-spill with a prescale of 3\",
                        \"enable\":false,
                        \"minc\" : \"0x18000\",
                        \"mexc\" : \"0x40\",
                        \"prescale\" : \"0x3\"
                        },
                        { \"id\":\"HLT_6\",
                        \"description\": \"Test CRT trigger US/DS without beam consideration with no prescale\",
                        \"enable\":true,
                        \"minc\" : \"0x18000\",
                        \"mexc\" : \"0x0\",
                        \"prescale\" : \"0x1\"
                        },
                        { \"id\":\"HLT_7\",
                        \"description\": \"Low beam energy hadron trigger (no electrons), Cherenkov selection (C1=0,C2=0), no PDS, no CRT\",
                        \"enable\":false,
                        \"minc\" : \"0x2\",
                        \"mexc\" : \"0xC\",
                        \"prescale\" : \"0x1\"
                        },
                        { \"id\":\"HLT_8\",
                        \"description\": \"Crossing muons Jura side\",
                        \"enable\":false,
                        \"minc\" : \"0xC0000\",
                        \"mexc\" : \"0x0\",
                        \"prescale\" : \"0x1\"
                        },
                        { \"id\":\"HLT_9\",
                        \"description\": \"Crossing muons Saleve side\",
                        \"enable\":false,
                        \"minc\" : \"0x3000\",
                        \"mexc\" : \"0x0\",
                        \"prescale\" : \"0x1\"
                        },
                        { \"id\":\"HLT_10\",
                        \"description\": \"HV current limit threshold\",
                        \"enable\":true,
                        \"minc\" : \"0x80\",
                        \"mexc\" : \"0x0\",
                        \"prescale\" : \"0x1\"
                        },
                        { \"id\":\"HLT_11\",
                        \"description\": \"Ground plane signals\",
                        \"enable\":true,
                        \"minc\" : \"0x100\",
                        \"mexc\" : \"0x0\",
                        \"prescale\" : \"0x1\"
                        },
                        { \"id\":\"HLT_12\",
                        \"description\": \"Purity monitor signals\",
                        \"enable\":true,
                        \"minc\" : \"0x200\",
                        \"mexc\" : \"0x0\",
                        \"prescale\" : \"0x1\"
                        },
                        { \"id\":\"HLT_13\",
                        \"description\": \"Spare\",
                        \"enable\":false,
                        \"minc\" : \"0x0\",
                        \"mexc\" : \"0x0\",
                        \"prescale\" : \"0x0\"
                        },
                        { \"id\":\"HLT_14\",
                        \"description\": \"Spare\",
                        \"enable\":false,
                        \"minc\" : \"0x0\",
                        \"mexc\" : \"0x0\",
                        \"prescale\" : \"0x0\"
                        },
                        { \"id\":\"HLT_15\",
                        \"description\": \"Spare\",
                        \"enable\":false,
                        \"minc\" : \"0x0\",
                        \"mexc\" : \"0x0\",
                        \"prescale\" : \"0x0\"
                        },
                        { \"id\":\"HLT_16\",
                        \"description\": \"Spare\",
                        \"enable\":false,
                        \"minc\" : \"0x0\",
                        \"mexc\" : \"0x0\",
                        \"prescale\" : \"0x0\"
                        },
                        { \"id\":\"HLT_17\",
                        \"description\": \"Spare\",
                        \"enable\":false,
                        \"minc\" : \"0x0\",
                        \"mexc\" : \"0x0\",
                        \"prescale\" : \"0x0\"
                        }
                ]
                },
                \"subsystems\":{
                \"pds\":{
                        \"channel_mask\":\"0xFFFFFF\",
                        \"reshape_length\" : 5,
                        \"delays\":[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
                        \"dac_thresholds\":[2185,2194,2170,2176,2167,2200,2179,2197,2188,2179,2185,2191,2176,2182,2191,2182,2176,2173,2200,2185,2182,2176,2191,2167],
                        \"triggers\": [
                        { \"id\":\"LLT_14\",
                        \"description\": \"PDS at least 3 channels (on Jura side)\",
                        \"enable\":false,
                        \"mask\" : \"0xF\",
                        \"type\" : \"0x1\",
                        \"count\" : \"0x2\"
                        },
                        { \"id\":\"LLT_17\",                   
                        \"description\": \"PDS include 2 SSPs near the beam plug\",
                        \"enable\":false,
                        \"mask\" : \"0x3\",
                        \"type\" : \"0x2\",
                        \"count\" : \"0x2\"
                        },
                        { \"id\":\"LLT_22\",                   
                        \"description\": \"PDS at least 10 SSPs firing overall. The clibration input is masked out\",
                        \"enable\":false,
                        \"mask\" : \"0xFFDFFF\",
                        \"type\" : \"0x1\",
                        \"count\" : \"0x9\"
                        },
                        { \"id\":\"LLT_23\",                   
                        \"description\": \"Spare\",
                        \"enable\":false,
                        \"mask\" : \"0x3\",
                        \"type\" : \"0x2\",
                        \"count\" : \"0x2\"
                        },
                        { \"id\":\"LLT_24\",                   
                        \"description\": \"Spare\",
                        \"enable\":false,
                        \"mask\" : \"0x3\",
                        \"type\" : \"0x2\",
                        \"count\" : \"0x2\"
                        }
                        ]
                },
                \"crt\":{
                        \"channel_mask\": \"0xFFFFFFFF\",
                        \"pixelate\": true,
                        \"reshape_length\" : 5,
                        \"delays\":[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],
                        \"triggers\": [
                        { \"id\":\"LLT_11\",
                        \"description\": \"Select CRT pixels around beam pipe (upstream)\",
                        \"enable\":false,
                        \"mask\" : \"0xF000\",
                        \"type\" : \"0x1\",
                        \"count\" : \"0x0\"
                        },
                        { \"id\":\"LLT_12\",
                        \"description\": \"Any upstream CRT Saleve side\",
                        \"enable\":false,
                        \"mask\" : \"0x60FC\",
                        \"type\" : \"0x1\",
                        \"count\" : \"0x0\"
                        },
                        { \"id\":\"LLT_13\",
                        \"description\": \"Any downstream CRT Saleve side \",
                        \"enable\":false,
                        \"mask\" : \"0x9F030000\",
                        \"type\" : \"0x1\",
                        \"count\" : \"0x0\"
                        },
                        { \"id\":\"LLT_15\",
                        \"description\": \"Any upstream CRT \",
                        \"enable\":true,
                        \"mask\" : \"0x60FC\",
                        \"type\" : \"0x1\",
                        \"count\" : \"0x0\"
                        },
                        { \"id\":\"LLT_16\",
                        \"description\": \"Any downstream CRT \",
                        \"enable\":true,
                        \"mask\" : \"0xFFFF0000\",
                        \"type\" : \"0x1\",
                        \"count\" : \"0x0\"
                        },
                        { \"id\":\"LLT_18\",
                        \"description\": \"Spare\",
                        \"enable\":false,
                        \"mask\" : \"0x0\",
                        \"type\" : \"0x1\",
                        \"count\" : \"0x0\"
                        },
                        { \"id\":\"LLT_19\",
                        \"description\": \"Spare\",
                        \"enable\":false,
                        \"mask\" : \"0x0\",
                        \"type\" : \"0x1\",
                        \"count\" : \"0x0\"
                        },
                        { \"id\":\"LLT_20\",
                        \"description\": \"Spare\",
                        \"enable\":false,
                        \"mask\" : \"0x0\",
                        \"type\" : \"0x1\",
                        \"count\" : \"0x0\"
                        },
                        { \"id\":\"LLT_21\",
                        \"description\": \"Spare\",
                        \"enable\":false,
                        \"mask\" : \"0x0\",
                        \"type\" : \"0x1\",
                        \"count\" : \"0x0\"
                        }
                        ]
                },
                \"beam\":{
                        \"channel_mask\": \"0x17FB\",
                        \"reshape_length\" : 50,
                        \"delays\":[1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0],
                        \"triggers\": [
                        { \"id\":\"LLT_1\",
                        \"description\": \"Beam trigger\",
                        \"enable\":true,
                        \"mask\" : \"0x1E3\"
                        },
                        { \"id\":\"LLT_2\",
                        \"description\": \"Mask in High Pressure Cherenkov (C713)\",
                        \"enable\":true,
                        \"mask\" : \"0x8\"
                        },
                        { \"id\":\"LLT_3\",
                        \"description\": \"Mask in Low Pressure Cherenkov (C716)\",
                        \"enable\":true,
                        \"mask\" : \"0x10\"
                        },
                        { \"id\":\"LLT_4\",
                        \"description\": \"Spare\",
                        \"enable\":false,
                        \"mask\" : \"0x0\"
                        },
                        { \"id\":\"LLT_5\",
                        \"description\": \"Spare\",
                        \"enable\":false,
                        \"mask\" : \"0x0\"
                        },
                        { \"id\":\"LLT_6\",
                        \"description\": \"Beam gate selection\",
                        \"enable\":true,
                        \"mask\" : \"0x2\"
                        },
                        { \"id\":\"LLT_7\",
                        \"description\": \"HV current limit threshold (init stream)\",
                        \"enable\":true,
                        \"mask\" : \"0x200\"
                        },
                        { \"id\":\"LLT_8\",
                        \"description\": \"Ground plane signals\",
                        \"enable\":true,
                        \"mask\" : \"0x400\"
                        },
                        { \"id\":\"LLT_9\",
                        \"description\": \"Purity monitor\",
                        \"enable\":true,
                        \"mask\" : \"0x1000\"
                        },
                        { \"id\":\"LLT_10\",
                        \"description\": \"Spare\",
                        \"enable\":false,
                        \"mask\" : \"0x0\"
                        },
                        { \"id\":\"LLT_25\",
                        \"description\": \"Spare\",
                        \"enable\":false,
                        \"mask\" : \"0x0\"
                        },
                        { \"id\":\"LLT_26\",
                        \"description\": \"Spare\",
                        \"enable\":false,
                        \"mask\" : \"0x0\"
                        }
                        ]
                }
                }', doc="CTB Configuration"),

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
