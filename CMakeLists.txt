cmake_minimum_required(VERSION 3.12)
project(ctbmodules VERSION 1.0.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(rcif REQUIRED)
find_package(appfwk REQUIRED)
find_package(opmonlib REQUIRED)
find_package(logging REQUIRED)
find_package(ers REQUIRED)

daq_codegen(ctbmodule.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2 )
daq_codegen(ctbmoduleinfo.jsonnet DEP_PKGS opmonlib TEMPLATES opmonlib/InfoStructs.hpp.j2 opmonlib/InfoNljs.hpp.j2 )


daq_add_plugin(CTBModule duneDAQModule            LINK_LIBRARIES appfwk::appfwk)

daq_install()
