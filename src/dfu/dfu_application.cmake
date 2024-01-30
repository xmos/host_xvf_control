# Building main application here

include(FetchContent)

# Fetch and build YAML parser
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG 0.8.0 # Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
)
FetchContent_GetProperties(yaml-cpp)
if(NOT yaml-cpp_POPULATED)
  FetchContent_Populate(yaml-cpp)
  add_subdirectory(${yaml-cpp_SOURCE_DIR} ${yaml-cpp_BINARY_DIR})
endif()

set( APP_NAME  xvf_dfu )

set(COMMON_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/dfu_main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dfu_commands.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dfu_operations.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../utils/utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../utils/platform_support.cpp
)
set(COMMON_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/../utils
    ${CMAKE_CURRENT_LIST_DIR}/../device
    ${DEVICE_CONTROL_PATH}/api
)

add_executable( ${APP_NAME})

target_compile_options( ${APP_NAME}
    PRIVATE
        -Werror
        -g
)

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

target_link_libraries( ${APP_NAME}
    PUBLIC
        dl
        yaml-cpp::yaml-cpp
)

target_link_options( ${APP_NAME}
    PRIVATE
        -rdynamic
)

# Copy YAML file to folder with app binary
add_custom_target(compile_commands ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_LIST_DIR}/dfu_cmds.yaml
    ${CMAKE_CURRENT_LIST_DIR}/transport_config.yaml
    $<TARGET_FILE_DIR:${APP_NAME}>
    COMMENT "Copy YAML files"
    VERBATIM
)
