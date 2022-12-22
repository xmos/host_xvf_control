# Building host device_control drivers here
# I2C and SPI drivers are only built for PI
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL armv7l)

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

add_library(device_rpi_i2c SHARED)
target_sources(device_rpi_i2c
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/device/device_i2c.cpp
)
target_include_directories(device_rpi_i2c
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/device
        ${DEVICE_CONTROL_PATH}/host
)
target_link_libraries(device_rpi_i2c
    PUBLIC  
        rtos::sw_services::device_control_host_i2c
)
target_link_options(device_rpi_i2c PRIVATE -fPIC)

add_library(device_rpi_spi SHARED)
target_sources(device_rpi_spi
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/device/device_spi.cpp
)
target_include_directories(device_rpi_spi
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/device
        ${DEVICE_CONTROL_PATH}/host
)
target_link_libraries(device_rpi_spi
    PUBLIC  
        rtos::sw_services::device_control_host_spi
)
target_link_libraries(device_rpi_spi PRIVATE -fPIC)

endif()

