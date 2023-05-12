# Building host device_control drivers here
# I2C and SPI drivers are only built for PI
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL armv7l)

# Build a wrapper driver for i2c

add_library(low_level_test_device_i2c SHARED)
target_sources(low_level_test_device_i2c
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/device/device_i2c.cpp
)
target_include_directories(low_level_test_device_i2c
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/device
        ${DEVICE_CONTROL_PATH}/host
)
target_compile_definitions(low_level_test_device_i2c
    PUBLIC
        LOW_LEVEL_TESTING=1
)
target_link_libraries(low_level_test_device_i2c
    PUBLIC  
        rtos::sw_services::device_control_host_i2c
)
target_link_options(low_level_test_device_i2c PRIVATE -fPIC)

# Build a wrapper driver for spi

add_library(low_level_test_device_spi SHARED)
target_sources(low_level_test_device_spi
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/device/device_spi.cpp
)
target_include_directories(low_level_test_device_spi
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/device
        ${DEVICE_CONTROL_PATH}/host
)
target_compile_definitions(low_level_test_device_spi
    PUBLIC
        LOW_LEVEL_TESTING=1
)
target_link_libraries(low_level_test_device_spi
    PUBLIC  
        rtos::sw_services::device_control_host_spi
)
target_link_libraries(low_level_test_device_spi PRIVATE -fPIC)

endif()
