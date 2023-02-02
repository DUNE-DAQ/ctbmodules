#!/usr/bin/env python3

import json
import os
import math
import sys
import glob
import rich.traceback
from rich.console import Console
from os.path import exists, join
from daqconf.core.system import System
from daqconf.core.conf_utils import make_app_command_data
from daqconf.core.metadata import write_metadata_file
from daqconf.core.config_file import generate_cli_from_schema

# Add -h as default help option
CONTEXT_SETTINGS = dict(help_option_names=['-h', '--help'])

console = Console()

# Set moo schema search path
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

import click

@click.command(context_settings=CONTEXT_SETTINGS)
@generate_cli_from_schema('ctbmodules/confgen.jsonnet', 'ctbmodules_gen')
@click.argument('json_dir', type=click.Path())

def cli(config, json_dir):

    if exists(json_dir):
        raise RuntimeError(f"Directory {json_dir} already exists")

    config_data = config[0]
    config_file = config[1]

    console.log('Loading cardcontrollerapp config generator')
    from ctbmodules.boardcontrollerapp import boardcontrollerapp_gen
 
    the_system = System()

    moo.otypes.load_types('ctbmodules/confgen.jsonnet')
    import dunedaq.ctbmodules.confgen as confgen
    moo.otypes.load_types('daqconf/confgen.jsonnet')
    import dunedaq.daqconf.confgen as daqconf

    nickname = 'ctb'
    console.log('generating cardcontrollerapp')

    boot = daqconf.boot(**config_data.boot)
   
    app = boardcontrollerapp_gen.get_boardcontroller_app(
        nickname = nickname,
    )
    console.log('generated cardcontrollerapp')
    the_system.apps[nickname] = app
    if boot.use_k8s:
        the_system.apps[nickname].resources = {
            #"felix.cern/flx0-ctrl": "1", # requesting FLX0 - modify for CTB
        }

    ####################################################################
    # Application command data generation
    ####################################################################

    from daqconf.core.conf_utils import make_app_command_data
    # Arrange per-app command data into the format used by util.write_json_files()
    app_command_datas = {
        name : make_app_command_data(the_system, app, name)
        for name,app in the_system.apps.items()
    }

    # Make boot.json config
    from daqconf.core.conf_utils import make_system_command_datas, write_json_files
    system_command_datas = make_system_command_datas(
        boot,
        the_system,
    )

    write_json_files(app_command_datas, system_command_datas, json_dir)

    console.log(f"CTB controller apps config generated in {json_dir}")

    write_metadata_file(json_dir, "ctbcontrollers_gen",config_file)

if __name__ == '__main__':
    try:
        cli(show_default=True, standalone_mode=True)
    except Exception as e:
        console.print_exception()
