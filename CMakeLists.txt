## Rasbian version that we currently use 3.13  is the latest possible version of cmake
## This cmake file will not be integrated with the top-level cmake structure till this is resolved
cmake_minimum_required(VERSION 3.13)
project(xvf3800_host_app)

if(NOT CMAKE_SYSTEM_NAME STREQUAL XCORE_XS3A)
    set(FWK_RTOS ${CMAKE_SOURCE_DIR}/fwk_rtos)
    enable_language(CXX C)
    add_subdirectory(src)
endif()