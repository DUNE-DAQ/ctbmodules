// This is the configuration schema for timinglibs

local moo = import "moo.jsonnet";

local stypes = import "daqconf/types.jsonnet";
local types = moo.oschema.hier(stypes).dunedaq.daqconf.types;

local sboot = import "daqconf/bootgen.jsonnet";
local bootgen = moo.oschema.hier(sboot).dunedaq.daqconf.bootgen;

local ns = "dunedaq.ctbmodules.confgen";
local s = moo.oschema.schema(ns);

// A temporary schema construction context.
local cs = {

  ctbmodules_gen: s.record('ctbmodules_gen', [
    s.field('boot',     bootgen.boot, default=bootgen.boot, doc='Boot parameters'),
  ]),

};

// Output a topologically sorted array.
stypes + sboot + moo.oschema.sort_select(cs, ns)
