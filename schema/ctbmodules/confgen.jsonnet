// This is the configuration schema for timinglibs

local moo = import "moo.jsonnet";
local sdc = import "daqconf/confgen.jsonnet";
local daqconf = moo.oschema.hier(sdc).dunedaq.daqconf.confgen;

local ns = "dunedaq.ctbmodules.confgen";
local s = moo.oschema.schema(ns);

// A temporary schema construction context.
local cs = {
  number: s.number  ("number", "i8", doc="a number"), // !?!?!

  ctbmodules_gen: s.record('ctbmodules_gen', [
    s.field('boot',                      daqconf.boot,                   default=daqconf.boot,                   doc='Boot parameters'),
  ]),

};

// Output a topologically sorted array.
sdc + moo.oschema.sort_select(cs, ns)
