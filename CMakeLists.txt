cmake_minimum_required(VERSION 3.12)

project(ctbmodules VERSION 3.0.1)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(rcif REQUIRED)
find_package(appfwk REQUIRED)
find_package(opmonlib REQUIRED)
find_package(logging REQUIRED)
find_package(ers REQUIRED)
find_package(hsilibs REQUIRED)

daq_add_plugin(CTBModule duneDAQModule            LINK_LIBRARIES hsilibs::hsilibs appfwk::appfwk)

daq_install()
