from pathlib import Path
import json

import conffwk

def update_ctb_settings(db_file:Path, jsonfile:Path, session_name:str|None) -> None :
    print (f"Setting {db_file} to {jsonfile}")

    json_data = json.load(open(jsonfile))

    db = conffwk.Configuration('oksconflibs:' + db_file)

    
