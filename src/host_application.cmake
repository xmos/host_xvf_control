# Building main application here

set(COMMON_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/types_support.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/platform_support.cpp
    ${CMAKE_CURRENT_LIST_DIR}/command/command.cpp
    ${CMAKE_CURRENT_LIST_DIR}/special_commands/special_commands.cpp
    ${CMAKE_CURRENT_LIST_DIR}/special_commands/filters.cpp
)
set(COMMON_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/utils
    ${CMAKE_CURRENT_LIST_DIR}/device
    ${CMAKE_CURRENT_LIST_DIR}/command
    ${CMAKE_CURRENT_LIST_DIR}/special_commands
    ${DEVICE_CONTROL_PATH}/api
)

add_executable(xvf_hostapp)

target_sources(xvf_hostapp
    PRIVATE
        ${COMMON_SOURCES}
)
target_include_directories(xvf_hostapp
    PUBLIC
        ${COMMON_INCLUDES}
)
target_link_libraries(xvf_hostapp
    PUBLIC 
        dl
)
target_link_options(xvf_hostapp
    PRIVATE
        -rdynamic
)

# If using clang disable c-linkage warning
if(HAVE_C_LINKAGE_WARNING)
    target_compile_options(xvf_hostapp PRIVATE -Wno-return-type-c-linkage)
endif()
