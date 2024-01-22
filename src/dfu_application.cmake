# Building main application here

include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG 0.8.0 # Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
)
FetchContent_GetProperties(yaml-cpp)

if(NOT yaml-cpp_POPULATED)
  message(STATUS "Fetching yaml-cpp...")
  FetchContent_Populate(yaml-cpp)
  add_subdirectory(${yaml-cpp_SOURCE_DIR} ${yaml-cpp_BINARY_DIR})
endif()

set( APP_NAME  xvf_dfu )

set(COMMON_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/dfu_main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/platform_support.cpp
)
set(COMMON_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/utils
    ${CMAKE_CURRENT_LIST_DIR}/device
    ${DEVICE_CONTROL_PATH}/api
)

add_executable( ${APP_NAME})

# Add options for different compilers
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options( ${APP_NAME}
        PRIVATE
            -WX
    )
else()
    target_compile_options( ${APP_NAME}
        PRIVATE
            -Werror
            -g
    )
endif()

target_sources( ${APP_NAME}
    PRIVATE
        ${COMMON_SOURCES}
)
target_include_directories( ${APP_NAME}
    PUBLIC
        ${COMMON_INCLUDES}
)
target_compile_definitions( ${APP_NAME}
    PRIVATE
        DEFAULT_DRIVER_NAME=device_i2c_dl_name
)

if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL Windows)
target_link_libraries(xvf_dfu
    PUBLIC
        dl
        yaml-cpp::yaml-cpp
)
target_link_options(xvf_dfu
    PRIVATE
        -rdynamic
)

endif() # not windows
