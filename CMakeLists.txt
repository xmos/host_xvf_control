## Rasbian version that we currently use 3.13  is the latest possible version of cmake
## This cmake file will not be integrated with the top-level cmake structure till this is resolved
cmake_minimum_required(VERSION 3.13)
project(host_xvf_control)

set(FWK_RTOS ${CMAKE_SOURCE_DIR}/fwk_rtos)
set(DEVICE_CONTROL_PATH ${FWK_RTOS}/modules/sw_services/device_control)
enable_language(CXX C)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)

include(src/host_drivers.cmake)
include(src/host_application.cmake)
