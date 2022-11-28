set(DEVICE_CONTROL_PATH ${FWK_RTOS}/modules/sw_services/device_control)

# Build device_control_host for I2C
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

# Link SPI driver
set(SPI_DRIVER ${CMAKE_CURRENT_LIST_DIR}/device/spi_driver)
add_library(bcm2835 STATIC IMPORTED)
set_property(TARGET bcm2835 PROPERTY IMPORTED_LOCATION ${SPI_DRIVER}/libbcm2835.a)
target_include_directories(bcm2835 INTERFACE ${SPI_DRIVER})

# Build device_control_host for SPI
add_library(framework_rtos_sw_services_device_control_host_spi INTERFACE)
target_sources(framework_rtos_sw_services_device_control_host_spi
    INTERFACE
        ${DEVICE_CONTROL_PATH}/host/util.c
        ${DEVICE_CONTROL_PATH}/host/device_access_spi_rpi.c
)
target_include_directories(framework_rtos_sw_services_device_control_host_spi
    INTERFACE
        ${DEVICE_CONTROL_PATH}/api
        ${DEVICE_CONTROL_PATH}/host
)

target_link_libraries(framework_rtos_sw_services_device_control_host_spi INTERFACE bcm2835)

target_compile_definitions(framework_rtos_sw_services_device_control_host_spi INTERFACE USE_SPI=1 RPI=1)
add_library(rtos::sw_services::device_control_host_spi ALIAS framework_rtos_sw_services_device_control_host_spi)

add_library(device_i2c_rpi SHARED)
target_sources(device_i2c_rpi
    PRIVATE
        device/device_i2c.cpp
)
target_include_directories(device_i2c_rpi
    PUBLIC
        device
        ${DEVICE_CONTROL_PATH}/host
)
target_link_libraries(device_i2c_rpi
    PUBLIC  
        rtos::sw_services::device_control_host_i2c
)
target_link_options(device_i2c_rpi PRIVATE -fPIC)

add_library(device_spi_rpi SHARED)
target_sources(device_spi_rpi
    PRIVATE
        device/device_spi.cpp
)
target_include_directories(device_spi_rpi
    PUBLIC
        device
        ${DEVICE_CONTROL_PATH}/host
)
target_link_libraries(device_spi_rpi
    PUBLIC  
        rtos::sw_services::device_control_host_spi
)
target_link_libraries(device_spi_rpi PRIVATE -fPIC)

if(TEST_BUILD)
    add_library(command_map SHARED command_map_dummy.cpp)
    target_include_directories(command_map PUBLIC ${DEVICE_CONTROL_PATH}/api)
    target_link_options(command_map PRIVATE -fPIC)
endif()

set(COMMON_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/command/command.cpp
    ${CMAKE_CURRENT_LIST_DIR}/special_commands/special_commands.cpp
    ${CMAKE_CURRENT_LIST_DIR}/device/factory.cpp
)

set(COMMON_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/device
    ${CMAKE_CURRENT_LIST_DIR}/command
    ${CMAKE_CURRENT_LIST_DIR}/special_commands
    ${DEVICE_CONTROL_PATH}/api
)


# SPI slave hostapp
add_executable(xvf_hostapp_rpi)

target_sources(xvf_hostapp_rpi
    PRIVATE
        ${COMMON_SOURCES}
)
target_include_directories(xvf_hostapp_rpi
    PUBLIC
        ${COMMON_INCLUDES}
)
target_link_libraries(xvf_hostapp_rpi
    PUBLIC 
        dl
)
target_link_options(xvf_hostapp_rpi
    PRIVATE
        -rdynamic
)

message(STATUS "Building with ${CMAKE_C_COMPILER_ID}")
