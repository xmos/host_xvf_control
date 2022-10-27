### Build device_control_host for i2c
set(DEVICE_CONTROL_PATH ${FWK_RTOS}/modules/sw_services/device_control)
### message(device_control_path="${DEVICE_CONTROL_PATH}")
add_library(framework_rtos_sw_services_device_control_host_i2c INTERFACE)
target_sources(framework_rtos_sw_services_device_control_host_i2c
    INTERFACE
        ${DEVICE_CONTROL_PATH}/host/util.c
        ${DEVICE_CONTROL_PATH}/host/device_access_i2c_rpi.c
)
target_include_directories(framework_rtos_sw_services_device_control_host_i2c
    INTERFACE
        ${DEVICE_CONTROL_PATH}/api
        ${DEVICE_CONTROL_PATH}/host
)
target_compile_definitions(framework_rtos_sw_services_device_control_host_i2c INTERFACE USE_I2C=1 RPI=1)
add_library(rtos::sw_services::device_control_host_i2c ALIAS framework_rtos_sw_services_device_control_host_i2c)

add_executable(application_xvf3800_int_host)

target_sources(application_xvf3800_int_host 
    PRIVATE 
        main.cpp
        command_map.cpp
        utils.cpp
        device/device_i2c.cpp
        command/command.cpp
        special_commands/special_commands.cpp
)

target_include_directories(application_xvf3800_int_host 
    PUBLIC
        .
        device
        command
        special_commands
)

target_link_libraries(application_xvf3800_int_host 
    PUBLIC 
        rtos::sw_services::device_control_host_i2c
)

message(STATUS "Building with ${CMAKE_C_COMPILER_ID}")