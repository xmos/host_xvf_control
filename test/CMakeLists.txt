
add_library(command_map SHARED command_map_dummy.cpp)
target_link_options(command_map PRIVATE -fPIC)
target_include_directories(command_map PUBLIC ${DEVICE_CONTROL_PATH}/api)

add_library(device_rpi_i2c SHARED device_dummy.cpp)
target_link_options(device_rpi_i2c PRIVATE -fPIC)
target_include_directories(device_rpi_i2c 
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/src/device
        ${DEVICE_CONTROL_PATH}/api
)

# If using clang disable c-linkage warning
if(HAVE_C_LINKAGE_WARNING)
    target_compile_options(device_rpi_i2c PRIVATE -Wno-return-type-c-linkage)
endif(HAVE_C_LINKAGE_WARNING)
