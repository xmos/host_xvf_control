## Rasbian version that we currently use 3.13  is the latest possible version of cmake
## This cmake file will not be integrated with the top-level cmake structure till this is resolved
cmake_minimum_required(VERSION 3.13)

option(TESTING "If set, cmake will build tests" OFF)
# Turn the option ON to use clang, you may need to change the path to your clang compiler
option(USE_CLANG "If set, cmake will use clang insted of gcc" OFF)
if(USE_CLANG)
    set(CMAKE_CXX_COMPILER "/usr/bin/clang++" CACHE STRING "C++ compiler" FORCE)
    set(CMAKE_C_COMPILER "/usr/bin/clang" CACHE STRING "C compiler" FORCE)
endif()

enable_language(CXX C)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)

# clang has a warning that can't be addressed, so it needs to be turned off if using clang
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wreturn-type-c-linkage HAVE_C_LINKAGE_WARNING)

message("CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")

project(host_xvf_control)

set(FWK_RTOS ${CMAKE_SOURCE_DIR}/fwk_rtos)
set(DEVICE_CONTROL_PATH ${FWK_RTOS}/modules/sw_services/device_control)

include(src/host_drivers.cmake)
include(src/host_application.cmake)

if(TESTING)
    add_subdirectory(test)
endif()
