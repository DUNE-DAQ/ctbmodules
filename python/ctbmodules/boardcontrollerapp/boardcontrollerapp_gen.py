# This module facilitates the generation of FLX card controller apps

# Set moo schema search path
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

# Load configuration types
import moo.otypes

moo.otypes.load_types('ctbmodules/ctbmodule.jsonnet')

# Import new types
import dunedaq.ctbmodules.ctbmodule as ctb

from daqconf.core.app import App, ModuleGraph
from daqconf.core.daqmodule import DAQModule

#===============================================================================
def get_boardcontroller_app(
        nickname,
):
    '''
    Here an entire application controlling one CTB board is generated. 
    '''

    # Define modules

    modules = []
    lus = []
    # Prepare standard config with no additional configuration
    modules += [DAQModule(name = nickname, 
                          plugin = 'CTBModule',
                          conf = ctb.Conf()
                             )]

    mgraph = ModuleGraph(modules)
    ctb_app = App(modulegraph=mgraph, name=nickname)

    return ctb_app