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
    
    uint8 : s.number("uint8", "u8",
                      doc="An 8 byte unsigned field"),

    string : s.string("String",
                       doc="A string field"),

    conf: s.record("Conf", [

        s.field("buffer_size", self.uint8, 0,
                doc="CTB Word Buffer Size"),

        s.field("receiver_connection_timeout", self.uint8, 0,
                doc="CTB Receiver Connection Timeout value (microseconds)"),

        s.field("control_connection_port", self.uint8, 0,
                doc="CTB Control Connection Port"),

        s.field("receiver_connection_port", self.uint8, 0,
                doc="CTB Receiver Connection Port"),

        s.field("ctb_hostname", self.string, "",
                doc="CTB Hostname"),

    ], doc="Central Trigger Board DAQ Module Configuration"),

};

moo.oschema.sort_select(ctbmodule, ns)
