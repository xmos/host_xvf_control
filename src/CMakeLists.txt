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

add_library(command_map SHARED command_map.cpp)
target_include_directories(command_map PUBLIC ${DEVICE_CONTROL_PATH}/api)
target_link_options(command_map PRIVATE -fPIC)

set(COMMON_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/command/command.cpp
    ${CMAKE_CURRENT_LIST_DIR}/special_commands/special_commands.cpp
)

set(COMMON_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/device
    ${CMAKE_CURRENT_LIST_DIR}/command
    ${CMAKE_CURRENT_LIST_DIR}/special_commands
)

# I2C slave hostapp
add_executable(xvf_host_rpi_i2c)

target_sources(xvf_host_rpi_i2c 
    PRIVATE
        ${COMMON_SOURCES}
        device/device_i2c.cpp

)

target_include_directories(xvf_host_rpi_i2c 
    PUBLIC
        ${COMMON_INCLUDES}
)

target_link_libraries(xvf_host_rpi_i2c 
    PUBLIC 
        rtos::sw_services::device_control_host_i2c
        dl
)

# SPI slave hostapp
add_executable(xvf_host_rpi_spi)

target_sources(xvf_host_rpi_spi
    PRIVATE
        ${COMMON_SOURCES}
        device/device_spi.cpp
)

target_include_directories(xvf_host_rpi_spi
    PUBLIC
        ${COMMON_INCLUDES}
)

target_link_libraries(xvf_host_rpi_spi
    PUBLIC 
        rtos::sw_services::device_control_host_spi
        dl
)

message(STATUS "Building with ${CMAKE_C_COMPILER_ID}")
