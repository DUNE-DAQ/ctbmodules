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

def update_triggers(updated_triggers, default_trigger_conf):
    """
    Update or add HLT,LLT defintions to the defaults in the schema
    :param (List) updated_triggers: List of JSON LLT triggers configurations, can be None
    :param (List) default_trigger_conf: List of default LLTs defined in the MOO schema
    :return: (List) default or updated list of LLTs
    """
    if updated_triggers is None:
        return default_trigger_conf

    default_triggers = {trig["id"] : idx for idx, trig in enumerate(default_trigger_conf)}
    for new_trig in updated_triggers:
        if new_trig["id"] in default_triggers.keys():
            default_trigger_conf[default_triggers[new_trig["id"]]] = new_trig
        else:
            default_trigger_conf.append(new_trig)

    return default_trigger_conf

def update_array(updated_conf, default_conf):
    """
    Update an array in the default schema, if needed
    :param (List) updated_conf: Array of values to update
    :param (List) default_conf: Array from schema that needs to be updated.
    :return: (List) default or updated array
    """
    if updated_conf is not None and len(updated_conf) != 0:
        assert len(updated_conf) == len(default_conf), \
        ("CTBModule: Configured array is being overwritten by array of different length: "
        f"Default array has length {len(default_conf)}, incoming has length {len(updated_conf)}")
        default_conf = updated_conf
    return default_conf


def get_ctb_hsi_app(
        ctb_hsi,
        nickname,
        LLT_SOURCE_ID,
        HLT_SOURCE_ID,
        QUEUE_POP_WAIT_MS=10,
        LATENCY_BUFFER_SIZE=100000,
        DATA_REQUEST_TIMEOUT=1000,
):
    '''
    Here an entire application controlling one CTB board is generated. 
    '''

    # Temp variables - Remove
    HOST=ctb_hsi.host_ctb_hsi
    HLT_LIST=ctb_hsi.hlt_triggers
    BEAM_LLT_LIST=ctb_hsi.beam_llt_triggers
    CRT_LLT_LIST=ctb_hsi.crt_llt_triggers
    PDS_LLT_LIST=ctb_hsi.pds_llt_triggers
    FAKE_TRIG_1=ctb_hsi.fake_trig_1
    FAKE_TRIG_2=ctb_hsi.fake_trig_2
    BEAM_RESHAPES = ctb_hsi.beam_reshape_lengths
    CRT_RESHAPES = ctb_hsi.crt_reshape_lengths
    PDS_RESHAPES = ctb_hsi.pds_reshape_lengths
    BEAM_DELAYS = ctb_hsi.beam_delays
    CRT_DELAYS = ctb_hsi.crt_delays
    PDS_DELAYS = ctb_hsi.pds_delays

    console = Console()

    # Define modules

    modules = []
    lus = []
    # Prepare standard config with no additional configuration
    console.log('generating DAQ module')

    # Get default LLT and HLTs
    hlt_conf = ctb.Hlt().pod()
    beam_conf = ctb.Beam().pod()
    crt_conf = ctb.Crt().pod()
    pds_conf = ctb.Pds().pod()
    fake_triggers = ctb.Misc().pod()

    # Update LLT, HLTs with new or redefined triggers
    updated_hlt_triggers = update_triggers(updated_triggers=HLT_LIST, default_trigger_conf=hlt_conf["trigger"])
    updated_beam_triggers = update_triggers(updated_triggers=BEAM_LLT_LIST, default_trigger_conf=beam_conf["triggers"])
    updated_crt_triggers = update_triggers(updated_triggers=CRT_LLT_LIST, default_trigger_conf=crt_conf["triggers"])
    updated_pds_triggers = update_triggers(updated_triggers=PDS_LLT_LIST, default_trigger_conf=pds_conf["triggers"])

    # Accept top config level fake trigger definition
    fake_trig_1 = fake_triggers["randomtrigger_1"]
    fake_trig_2 = fake_triggers["randomtrigger_2"]
    if FAKE_TRIG_1 is not None:
        fake_trig_1 = FAKE_TRIG_1
    if FAKE_TRIG_2 is not None:
        fake_trig_2 = FAKE_TRIG_2

    # Accept top config level reshape and delay definitions
    updated_beam_reshapes = update_array(BEAM_RESHAPES, beam_conf["reshape_lengths"])
    updated_crt_reshapes = update_array(CRT_RESHAPES, crt_conf["reshape_lengths"])
    updated_pds_reshapes = update_array(PDS_RESHAPES, pds_conf["reshape_lengths"])
    updated_beam_delays = update_array(BEAM_DELAYS, beam_conf["delays"])
    updated_crt_delays = update_array(CRT_DELAYS, crt_conf["delays"])
    updated_pds_delays = update_array(PDS_DELAYS, pds_conf["delays"])
    
    updated_pds_conf = ctb.Pds(triggers=updated_pds_triggers, 
                               reshape_lengths=updated_pds_reshapes,
                               delays=updated_pds_delays)
    updated_crt_conf = ctb.Crt(triggers=updated_crt_triggers, 
                               reshape_lengths=updated_crt_reshapes,
                               delays=updated_crt_delays)
    updated_beam_conf = ctb.Beam(triggers=updated_beam_triggers, 
                               reshape_lengths=updated_beam_reshapes,
                               delays=updated_beam_delays)
    modules += [DAQModule(name = nickname, 
                          plugin = 'CTBModule',
                          conf = ctb.Conf(board_config=ctb.Board_config(ctb=ctb.Ctb(misc=ctb.Misc(randomtrigger_1=fake_trig_1, randomtrigger_2=fake_trig_2),
                                HLT=ctb.Hlt(trigger=updated_hlt_triggers),
                                subsystems=ctb.Subsystems(pds=updated_pds_conf,
                                                          crt=updated_crt_conf,
                                                          beam=updated_beam_conf),
                                sockets=ctb.Sockets(receiver=ctb.Receiver(host=HOST)) 
                                )))
                             )]


    modules += [DAQModule(name = f"ctb_llt_datahandler",
                        plugin = "HSIDataLinkHandler",
                        conf = rconf.Conf(readoutmodelconf = rconf.ReadoutModelConf(source_queue_timeout_ms = QUEUE_POP_WAIT_MS, 
                                                                                    source_id=LLT_SOURCE_ID,
                                                                                    send_partial_fragment_if_available = True),
                                          latencybufferconf = rconf.LatencyBufferConf(latency_buffer_size = LATENCY_BUFFER_SIZE),
                                          rawdataprocessorconf = rconf.RawDataProcessorConf(source_id=LLT_SOURCE_ID),
                                          requesthandlerconf= rconf.RequestHandlerConf(latency_buffer_size = LATENCY_BUFFER_SIZE,
                                                                                          pop_limit_pct = 0.8,
                                                                                          pop_size_pct = 0.1,
                                                                                          source_id=LLT_SOURCE_ID,
                                                                                          # output_file = f"output_{idx + MIN_LINK}.out",
                                                                                          request_timeout_ms = DATA_REQUEST_TIMEOUT,
                                                                                          warn_about_empty_buffer = False,
                                                                                          enable_raw_recording = False)
                                             ))]
                                             
    modules += [DAQModule(name = f"ctb_hlt_datahandler",
        plugin = "HSIDataLinkHandler",
        conf = rconf.Conf(readoutmodelconf = rconf.ReadoutModelConf(source_queue_timeout_ms = QUEUE_POP_WAIT_MS,
                                                                source_id=HLT_SOURCE_ID,
                                                                send_partial_fragment_if_available = True),
                        latencybufferconf = rconf.LatencyBufferConf(latency_buffer_size = LATENCY_BUFFER_SIZE),
                        rawdataprocessorconf = rconf.RawDataProcessorConf(source_id=HLT_SOURCE_ID),
                        requesthandlerconf= rconf.RequestHandlerConf(latency_buffer_size = LATENCY_BUFFER_SIZE,
                                                                        pop_limit_pct = 0.8,
                                                                        pop_size_pct = 0.1,
                                                                        source_id=HLT_SOURCE_ID,
                                                                        # output_file = f"output_{idx + MIN_LINK}.out",
                                                                        request_timeout_ms = DATA_REQUEST_TIMEOUT,
                                                                        warn_about_empty_buffer = False,
                                                                        enable_raw_recording = False)
                        ))]

    queues = [Queue(f"{nickname}.llt_output",f"ctb_llt_datahandler.raw_input","HSIFrame",f'ctb_llt_link', 100000),Queue(f"{nickname}.hlt_output",f"ctb_hlt_datahandler.raw_input","HSIFrame",f'ctb_hlt_link', 100000)]

    mgraph = ModuleGraph(modules, queues=queues)
    
    mgraph.add_fragment_producer(id = LLT_SOURCE_ID, subsystem = "HW_Signals_Interface",
                                         requests_in   = f"ctb_llt_datahandler.request_input",
                                         fragments_out = f"ctb_llt_datahandler.fragment_queue")

    mgraph.add_fragment_producer(id = HLT_SOURCE_ID, subsystem = "HW_Signals_Interface",
                                         requests_in   = f"ctb_hlt_datahandler.request_input",
                                         fragments_out = f"ctb_hlt_datahandler.fragment_queue")

    mgraph.add_endpoint(f"timesync_ctb_llt", f"ctb_llt_datahandler.timesync_output", "TimeSync", Direction.OUT, is_pubsub=True, toposort=False)
    mgraph.add_endpoint(f"timesync_ctb_hlt", f"ctb_hlt_datahandler.timesync_output", "TimeSync", Direction.OUT, is_pubsub=True, toposort=False)

    mgraph.add_endpoint("ctb_hsievents", f"{nickname}.hsievents", "HSIEvent",    Direction.OUT)

    # dummy subscriber
    mgraph.add_endpoint(None, None, data_type="TimeSync", inout=Direction.IN, is_pubsub=True)

    console.log('generated DAQ module')
    ctb_app = App(modulegraph=mgraph, host=HOST, name=nickname)

    return ctb_app
