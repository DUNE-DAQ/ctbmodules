cmake_minimum_required(VERSION 3.12)

project(ctbmodules VERSION 4.0.1)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(appfwk REQUIRED)
find_package(opmonlib REQUIRED)
find_package(logging REQUIRED)
find_package(ers REQUIRED)
find_package(hsilibs REQUIRED)
find_package(daqconf REQUIRED)

daq_protobuf_codegen( opmon/*.proto )

daq_add_library( *.cpp LINK_LIBRARIES 
                 opmonlib::opmonlib ers::ers appfwk::appfwk logging::logging stdc++fs hsilibs::hsilibs )

daq_add_plugin(CTBModule duneDAQModule            LINK_LIBRARIES ctbmodules )

daq_install()
