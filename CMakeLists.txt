cmake_minimum_required(VERSION 3.12)
project(ctbmodules VERSION 2.0.1)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(rcif REQUIRED)
find_package(appfwk REQUIRED)
find_package(opmonlib REQUIRED)
find_package(logging REQUIRED)
find_package(ers REQUIRED)
find_package(hsilibs REQUIRED)

daq_codegen(ctbmodule.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2 )
daq_codegen(ctbmoduleinfo.jsonnet DEP_PKGS opmonlib TEMPLATES opmonlib/InfoStructs.hpp.j2 opmonlib/InfoNljs.hpp.j2 )


daq_add_plugin(CTBModule duneDAQModule            LINK_LIBRARIES hsilibs::hsilibs appfwk::appfwk)

daq_install()
