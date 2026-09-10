from pathlib import Path
import json

import conffwk
import daqconf.include as include

def update_ctb_settings(db_file:Path, jsonfile:Path, session_name:str|None) -> None :
    print (f"Setting {db_file} to {jsonfile}")

    json_data = json.load(open(jsonfile))
    ctb = json_data["ctb"]

    db = conffwk.Configuration('oksconflibs:' + db_file)
    set_sockets(db, ctb["sockets"])

    ## object to be included to be return in a list
    to_be_included = set_misc( db, ctb["misc"])
    to_be_included += set_hlts( db, ctb["HLT"]["trigger"])  ## Remember that the HLT block contains other parts which are not stored in our schema, so they are not configurable
    to_be_included += set_subsystems( db, ctb["subsystems"])

    db.commit()
    
    triggers = db.get_dals("CTBTrigger")
    all_triggers = [t.id for t in triggers]
    to_be_excluded = []
    for t in all_triggers :
        if t not in to_be_included :
            to_be_excluded.append(t)

    print("Including:",to_be_included)
    print("Excluding:",to_be_excluded)

    if not session_name :
        session_name = db.get_dals("Session")[0].id
        
    include.include(db_file, False, to_be_included, session_name)
    include.include(db_file, True, to_be_excluded, session_name)


def set_subsystems(db:conffwk.Configuration, systems:dict) -> list[str] :

    set_pds(db, systems["pds"])

    include = []
    include += set_crt(db, systems["crt"])
    include += set_beam(db, systems["beam"])
    return include

def set_beam(db:conffwk.Configuration, beam:dict) -> list[str] :

    board = db.get_dals("CTBoardConf")[0]  
    oks = board.beam
    oks.channel_mask = beam["channel_mask"]
    oks.reshape_lengths = beam["reshape_lengths"]
    oks.delays = beam["delays"]
    db.update_dal(oks)

    return set_beam_llts(db, beam["triggers"])


def set_beam_llts(db:conffwk.Configuration, triggers:dict) -> list[str] :

    include = []
    for t in triggers :
        oks = db.get_dal("CTBLLT", t["id"])
        oks.mask = t["mask"]
        oks.description = t["description"]
        db.update_dal(oks)
        if t["enable"] :
            include.append(t["id"])
    return include


def set_crt(db:conffwk.Configuration, crt:dict) -> list[str] :

    oks = db.get_dals("CTBCRTSubsystem")[0]
    oks.pixelate = crt["pixelate"]
    oks.channel_mask = crt["channel_mask"]
    oks.reshape_lengths = crt["reshape_lengths"]
    oks.delays = crt["delays"]
    db.update_dal(oks)

    return set_crt_llts(db, crt["triggers"])
    
def set_crt_llts(db:conffwk.Configuration, triggers:dict) -> list[str] :

    include = []
    for t in triggers :
        oks = db.get_dal("CTBCountLLT", t["id"])
        oks.count = t["count"]
        oks.type = t["type"]
        oks.mask = t["mask"]
        oks.description = t["description"]
        db.update_dal(oks)
        if t["enable"] :
            include.append(t["id"])
    return include
        

def set_pds(db:conffwk.Configuration, pds:dict) -> None :
    oks = db.get_dals("CTBPDSSubsystem")[0]
    oks.dac_thresholds = pds["dac_thresholds"]
    oks.channel_mask = pds["channel_mask"]
    oks.reshape_lengths = pds["reshape_lengths"]
    oks.delays = pds["delays"]
    db.update_dal(oks)

    set_pds_llts(db, pds["triggers"])
   

def set_pds_llts(db:conffwk.Configuration, triggers:dict) -> None :
    for t in triggers :
        oks = db.get_dal("CTBPDSLLT", t["id"])
        oks.id = t["id"]
        oks.description = t["description"]
        oks.enable = t["enable"]
        oks.mask= t["mask"]
        oks.type = t["mask"]
        oks.count = t["count"]
        db.update_dal(oks)
    
def set_hlts(db:conffwk.Configuration, hlts:dict) -> list[str] :

    include = []
    for d in hlts :
        set_hlt(db, d)
        if d["enable"] :
            include.append(d["id"])
            
    return include

def set_hlt(db:conffwk.Configuration, hlt:dict) -> None :
    oks = db.get_dal("CTBHLT", hlt["id"])
    oks.minc = hlt["minc"]
    oks.mexc = hlt["mexc"]
    oks.prescale = hlt["prescale"]
    oks.description = hlt["description"]
    db.update_dal(oks)
    
def set_misc(db:conffwk.Configuration, misc:dict) -> list[str] :

    oks_misc = db.get_dals("CTBMisc")[0]
    oks_misc.ch_status = misc["ch_status"]
    db.update_dal(oks_misc)

    pulser = misc["pulser"]
    oks_pulser = oks_misc.pulser
    oks_pulser.enable = pulser["enable"]
    oks_pulser.frequency = pulser["frequency"]
    db.update_dal(oks_pulser)

    timing = misc["timing"]
    oks_timing = oks_misc.timing
    oks_timing.address = timing["address"]
    oks_timing.triggers = timing["triggers"]
    oks_timing.lockout = timing["lockout"]

    db.update_dal(oks_timing)

    to_be_included = []
    to_be_included += set_random_trigger(db, "HLT_0", misc["randomtrigger_1"])
    to_be_included += set_random_trigger(db, "LLT_0", misc["randomtrigger_2"])

    return to_be_included

def set_random_trigger(db:conffwk.Configuration, trigger_id:str, trigger:dict) -> list[str] :
    oks_trigger=db.get_dal("CTBRandomTrigger", trigger_id)
    oks_trigger.fixed_freq=trigger["fixed_freq"]
    oks_trigger.beam_mode=trigger["beam_mode"]
    oks_trigger.period=trigger["period"]
    oks_trigger.description=trigger["description"]
    db.update_dal(oks_trigger)
        
    if trigger["enable"] :
        return [trigger_id]
    return []
    
    
def set_sockets(db:conffwk.Configuration, sockets:dict) -> None :

    oks_sockets = db.get_dals("CTBSockets")[0]
    
    oks_receiver = oks_sockets.receiver
    receiver = sockets["receiver"]
    oks_receiver.rollover = receiver["rollover"]
    oks_receiver.port = receiver["port"]
    db.update_dal(oks_receiver)
    
    oks_monitor = oks_sockets.monitor
    monitor = sockets["monitor"]
    oks_monitor.enable = monitor["enable"]
    oks_monitor.port = monitor["port"]
    db.update_dal(oks_monitor)
    
    oks_statistics = oks_sockets.statistics
    stat = sockets["statistics"]
    oks_statistics.enable= stat["enable"]
    oks_statistics.port = stat["port"]
    oks_statistics.updt_period = stat["updt_period"]
    db.update_dal(oks_statistics)

        
    
    
