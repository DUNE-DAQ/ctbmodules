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


def get_ctb_hsi_app(
        nickname,
        LLT_SOURCE_ID,
        HLT_SOURCE_ID,
        QUEUE_POP_WAIT_MS=10,
        LATENCY_BUFFER_SIZE=100000,
        DATA_REQUEST_TIMEOUT=1000,
        HOST="localhost",
        HLT_LIST=None,
        BEAM_LLT_LIST=None,
        CRT_LLT_LIST=None,
        PDS_LLT_LIST=None
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

    # Get default LLT and HLTs
    hlt_trig = ctb.Hlt().pod()
    beam_trig = ctb.Beam().pod()
    crt_trig = ctb.Crt().pod()
    pds_trig = ctb.Pds().pod()

    # Update LLT, HLTs with new or redefined triggers
    updated_hlt_triggers = update_triggers(updated_triggers=HLT_LIST, default_trigger_conf=hlt_trig["trigger"])
    updated_beam_triggers = update_triggers(updated_triggers=BEAM_LLT_LIST, default_trigger_conf=beam_trig["triggers"])
    updated_crt_triggers = update_triggers(updated_triggers=CRT_LLT_LIST, default_trigger_conf=crt_trig["triggers"])
    updated_pds_triggers = update_triggers(updated_triggers=PDS_LLT_LIST, default_trigger_conf=pds_trig["triggers"])

    modules += [DAQModule(name = nickname, 
                          plugin = 'CTBModule',
                          conf = ctb.Conf(hsievent_connection_name = "hsievents",
                                board_config=ctb.Board_config(ctb=ctb.Ctb(HLT=ctb.Hlt(trigger=updated_hlt_triggers),
                                subsystems=ctb.Subsystems(pds=ctb.Pds(triggers=updated_pds_triggers),
                                                          crt=ctb.Crt(triggers=updated_crt_triggers),
                                                          beam=ctb.Beam(triggers=updated_beam_triggers)) 
                                )))
                             )]


    modules += [DAQModule(name = f"ctb_llt_datahandler",
                        plugin = "HSIDataLinkHandler",
                        conf = rconf.Conf(readoutmodelconf = rconf.ReadoutModelConf(source_queue_timeout_ms = QUEUE_POP_WAIT_MS, 
                                                                                    source_id=LLT_SOURCE_ID,
                                                                                    send_partial_fragment_if_available = True),
                                          latencybufferconf = rconf.LatencyBufferConf(latency_buffer_size = LATENCY_BUFFER_SIZE, source_id=LLT_SOURCE_ID),
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
                        latencybufferconf = rconf.LatencyBufferConf(latency_buffer_size = LATENCY_BUFFER_SIZE,
                                                                        source_id=HLT_SOURCE_ID),
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

    mgraph.add_endpoint("hsievents", None,     Direction.OUT)
    mgraph.add_endpoint(None, None, Direction.IN, ["Timesync"])

    console.log('generated DAQ module')
    ctb_app = App(modulegraph=mgraph, name=nickname)

    return ctb_app
