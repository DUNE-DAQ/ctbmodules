# This module facilitates the generation of FLX card controller apps
from rich.console import Console
# Set moo schema search path
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

# Load configuration types
import moo.otypes

moo.otypes.load_types('ctbmodules/ctbmodule.jsonnet')
moo.otypes.load_types('readoutlibs/readoutconfig.jsonnet')

# Import new types
import dunedaq.ctbmodules.ctbmodule as ctb

import dunedaq.readoutlibs.readoutconfig as rconf

from daqconf.core.app import App, ModuleGraph
from daqconf.core.daqmodule import DAQModule

from daqconf.core.conf_utils import Direction, Queue

#===============================================================================
def get_boardcontroller_app(
        nickname,
        QUEUE_POP_WAIT_MS=10,
        LATENCY_BUFFER_SIZE=100000,
        DATA_REQUEST_TIMEOUT=1000,
):
    '''
    Here an entire application controlling one CTB board is generated. 
    '''
    console = Console()

    # Define modules

    modules = []
    lus = []
    # Prepare standard config with no additional configuration
    console.log('generating DAQ module')
    modules += [DAQModule(name = nickname, 
                          plugin = 'CTBModule',
                          #conf = ctb.Conf(board_config=ctb.Board_config(ctb=ctb.Ctb(misc=ctb.Misc(randomtrigger_1=ctb.Randomtrigger(description="Random trigger that can optionally be set to fire only during beam spill",enable=False,fixed_freq=True,beam_mode=True,period=100000),randomtrigger_2=ctb.Randomtrigger(description="Random trigger that can optionally be set to fire only during beam spill",enable=False,fixed_freq=True,beam_mode=True,period=100000)))))
                          conf = ctb.Conf(board_config=ctb.Board_config(ctb=ctb.Ctb(HLT=ctb.Hlt(trigger=[
                                ctb.Hlt_trigger(id="HLT_1",description="Reconstructable track beam trigger, no PDS selection, no CRT selection",enable=False,minc="0x2",mexc="0x0",prescale="0x1"),
                                ctb.Hlt_trigger(id="HLT_2",description="Reconstructable track beam trigger, particle selection with Cherenkov detectors (HP=1,LP=1), no PDS selection, no CRT selection",enable=False,minc="0xE",mexc="0x0",prescale="0x1"),
                                ctb.Hlt_trigger(id="HLT_3",description="Cherenkov particle selection with with C713=0 & C716=1, no CRT, no PDS",enable=False,minc="0xA",mexc="0x4",prescale="0x1"),
                                ctb.Hlt_trigger(id="HLT_4",description="Spare",enable=False,minc="0x0",mexc="0x0",prescale="0x1"),
                                ctb.Hlt_trigger(id="HLT_5",description="CRT trigger US/DS off-spill with a prescale of 3",enable=False,minc="0x18000",mexc="0x40",prescale="0x3"),
                                ctb.Hlt_trigger(id="HLT_6",description="Test CRT trigger US/DS without beam consideration with no prescale",enable=True,minc="0x18000",mexc="0x0",prescale="0x1"),
                                ctb.Hlt_trigger(id="HLT_7",description="Low beam energy hadron trigger (no electrons), Cherenkov selection (C1=0,C2=0), no PDS, no CRT",enable=False,minc="0x2",mexc="0xC",prescale="0x1"),
                                ctb.Hlt_trigger(id="HLT_8",description="Crossing muons Jura side",enable=False,minc="0xC0000",mexc="0x0",prescale="0x1"),
                                ctb.Hlt_trigger(id="HLT_9",description="Crossing muons Saleve side",enable=False,minc="0x3000",mexc="0x0",prescale="0x1"),
                                ctb.Hlt_trigger(id="HLT_10",description="HV current limit threshold",enable=True,minc="0x80",mexc="0x0",prescale="0x1"),
                                ctb.Hlt_trigger(id="HLT_11",description="Ground plane signals",enable=True,minc="0x100",mexc="0x0",prescale="0x1"),
                                ctb.Hlt_trigger(id="HLT_12",description="Purity monitor signals",enable=True,minc="0x200",mexc="0x0",prescale="0x1"),
                                ctb.Hlt_trigger(id="HLT_13",description="Spare",enable=False,minc="0x0",mexc="0x0",prescale="0x0"),
                                ctb.Hlt_trigger(id="HLT_14",description="Spare",enable=False,minc="0x0",mexc="0x0",prescale="0x0"),
                                ctb.Hlt_trigger(id="HLT_15",description="Spare",enable=False,minc="0x0",mexc="0x0",prescale="0x0"),
                                ctb.Hlt_trigger(id="HLT_16",description="Spare",enable=False,minc="0x0",mexc="0x0",prescale="0x0"),
                                ctb.Hlt_trigger(id="HLT_17",description="Spare",enable=False,minc="0x0",mexc="0x0",prescale="0x0")
                                ]),
                                subsystems=ctb.Subsystems(pds=ctb.Pds(triggers=[
                                        ctb.Llt_trigger(id="LLT_14",description="PDS at least 3 channels (on Jura side)", enable=False, mask="0xF", type="0x1", count="0x2"),
                                        ctb.Llt_trigger(id="LLT_17",description="PDS include 2 SSPs near the beam plug", enable=False, mask="0x3", type="0x2", count="0x2"),
                                        ctb.Llt_trigger(id="LLT_22",description="PDS at least 10 SSPs firing overall. The calibration input is masked out", enable=False, mask="0xFFDFFF", type="0x1", count="0x9"),
                                        ctb.Llt_trigger(id="LLT_23",description="Spare", enable=False, mask="0x3", type="0x2", count="0x2"),
                                        ctb.Llt_trigger(id="LLT_24",description="Spare", enable=False, mask="0x3", type="0x2", count="0x2")                                        
                                        ]),
                                        crt=ctb.Crt(triggers=[
                                        ctb.Llt_trigger(id="LLT_11",description="Select CRT pixels around beam pipe (upstream)", enable=False, mask="0xF000", type="0x1", count="0x0"),
                                        ctb.Llt_trigger(id="LLT_12",description="Any upstream CRT Saleve side", enable=False, mask="0x60FC", type="0x1", count="0x0"),
                                        ctb.Llt_trigger(id="LLT_13",description="Any downstream CRT Saleve side", enable=False, mask="0x9F030000", type="0x1", count="0x0"),
                                        ctb.Llt_trigger(id="LLT_15",description="Any upstream CRT", enable=True, mask="0x60FC", type="0x1", count="0x0"),
                                        ctb.Llt_trigger(id="LLT_16",description="Any downstream CRT", enable=True, mask="0xFFFF0000", type="0x1", count="0x0"),
                                        ctb.Llt_trigger(id="LLT_18",description="Spare", enable=False, mask="0x0", type="0x1", count="0x0"),                                        
                                        ctb.Llt_trigger(id="LLT_19",description="Spare", enable=False, mask="0x0", type="0x1", count="0x0"),                                        
                                        ctb.Llt_trigger(id="LLT_20",description="Spare", enable=False, mask="0x0", type="0x1", count="0x0"),                                        
                                        ctb.Llt_trigger(id="LLT_21",description="Spare", enable=False, mask="0x0", type="0x1", count="0x0")                                        
                                        ]),
                                        beam=ctb.Beam(triggers=[
                                        ctb.Llt_trigger_red(id="LLT_1",description="Beam trigger", enable=True, mask="0x1E3"),
                                        ctb.Llt_trigger_red(id="LLT_2",description="Mask in High Pressure Cherenkov (C713)", enable=True, mask="0x8"),
                                        ctb.Llt_trigger_red(id="LLT_3",description="Mask in Low Pressure Cherenkov (C716)", enable=True, mask="0x10"),
                                        ctb.Llt_trigger_red(id="LLT_4",description="Spare", enable=False, mask="0x0"),
                                        ctb.Llt_trigger_red(id="LLT_5",description="Spare", enable=False, mask="0x0"),
                                        ctb.Llt_trigger_red(id="LLT_6",description="Beam gate selection", enable=True, mask="0x2"),                                        
                                        ctb.Llt_trigger_red(id="LLT_7",description="HV current limit threshold (init stream)", enable=True, mask="0x200"),                                        
                                        ctb.Llt_trigger_red(id="LLT_8",description="Ground plane signals", enable=True, mask="0x400"),                                        
                                        ctb.Llt_trigger_red(id="LLT_9",description="Purity monitor", enable=True, mask="0x1000"),
                                        ctb.Llt_trigger_red(id="LLT_10",description="Spare", enable=False, mask="0x0"),
                                        ctb.Llt_trigger_red(id="LLT_25",description="Spare", enable=False, mask="0x0"),                                        
                                        ctb.Llt_trigger_red(id="LLT_26",description="Spare", enable=False, mask="0x0")                                                                               
                                        ])) 
                                )))
                             )]

    modules += [DAQModule(name = f"ctb_llt_datahandler",
                        plugin = "HSIDataLinkHandler",
                        conf = rconf.Conf(readoutmodelconf = rconf.ReadoutModelConf(source_queue_timeout_ms = QUEUE_POP_WAIT_MS, send_partial_fragment_if_available = True),
                                          latencybufferconf = rconf.LatencyBufferConf(latency_buffer_size = LATENCY_BUFFER_SIZE, source_id=0),
                                          rawdataprocessorconf = rconf.RawDataProcessorConf(source_id=0),
                                          requesthandlerconf= rconf.RequestHandlerConf(latency_buffer_size = LATENCY_BUFFER_SIZE,
                                                                                          pop_limit_pct = 0.8,
                                                                                          pop_size_pct = 0.1,
                                                                                          source_id=0,
                                                                                          # output_file = f"output_{idx + MIN_LINK}.out",
                                                                                          request_timeout_ms = DATA_REQUEST_TIMEOUT,
                                                                                          warn_about_empty_buffer = False,
                                                                                          enable_raw_recording = False)
                                             ))]
                                             
    modules += [DAQModule(name = f"ctb_hlt_datahandler",
        plugin = "HSIDataLinkHandler",
        conf = rconf.Conf(readoutmodelconf = rconf.ReadoutModelConf(source_queue_timeout_ms = QUEUE_POP_WAIT_MS,
                                                                source_id=1,
                                                                send_partial_fragment_if_available = True),
                        latencybufferconf = rconf.LatencyBufferConf(latency_buffer_size = LATENCY_BUFFER_SIZE,
                                                                        source_id=1),
                        rawdataprocessorconf = rconf.RawDataProcessorConf(source_id=1),
                        requesthandlerconf= rconf.RequestHandlerConf(latency_buffer_size = LATENCY_BUFFER_SIZE,
                                                                        pop_limit_pct = 0.8,
                                                                        pop_size_pct = 0.1,
                                                                        source_id=1,
                                                                        # output_file = f"output_{idx + MIN_LINK}.out",
                                                                        request_timeout_ms = DATA_REQUEST_TIMEOUT,
                                                                        warn_about_empty_buffer = False,
                                                                        enable_raw_recording = False)
                        ))]

    queues = [Queue(f"ctb.llt_output",f"ctb_llt_datahandler.raw_input",f'ctb_llt_link', 100000),Queue(f"ctb.hlt_output",f"ctb_hlt_datahandler.raw_input",f'ctb_hlt_link', 100000)]

    mgraph = ModuleGraph(modules, queues=queues)
    
    mgraph.add_fragment_producer(id = 0, subsystem = "HW_Signals_Interface",
                                         requests_in   = f"ctb_llt_datahandler.request_input",
                                         fragments_out = f"ctb_llt_datahandler.fragment_queue")

    mgraph.add_fragment_producer(id = 1, subsystem = "HW_Signals_Interface",
                                         requests_in   = f"ctb_hlt_datahandler.request_input",
                                         fragments_out = f"ctb_hlt_datahandler.fragment_queue")

    mgraph.add_endpoint(f"timesync_ctb_llt", f"ctb_llt_datahandler.timesync_output",    Direction.OUT, ["Timesync"], toposort=False)
    mgraph.add_endpoint(f"timesync_ctb_hlt", f"ctb_hlt_datahandler.timesync_output",    Direction.OUT, ["Timesync"], toposort=False)

    mgraph.add_endpoint(None, None, Direction.IN, ["Timesync"])

    console.log('generated DAQ module')
    ctb_app = App(modulegraph=mgraph, name=nickname)

    return ctb_app
