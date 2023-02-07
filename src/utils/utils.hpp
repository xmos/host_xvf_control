// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef COMMAND_UTILS_H_
#define COMMAND_UTILS_H_

#include <cstring>
#include <cstdint>
#include "device.hpp"

#if defined(_WIN32)
#include <string>
#endif

/** @brief Enum for read/write command types */
enum cmd_rw_t {CMD_RO, CMD_WO, CMD_RW};

/**
 * @brief Enum for supported param types
 * 
 * @note Add new cmd_param_type's to the end of the list.
 * @note TYPE_CHAR can only be READ ONLY.
 */
enum cmd_param_type_t {TYPE_CHAR, TYPE_UINT8, TYPE_INT32, TYPE_FLOAT, TYPE_UINT32, TYPE_RADIANS};

/** @brief Union for supporting different command param types */
union cmd_param_t {uint8_t ui8; int32_t i32; float f; uint32_t ui32;};

/** @brief Command configuration structure
 * 
 * @note cmd_name has to be upper case
 */
struct cmd_t
{
    /** Command resource ID */
    control_resid_t res_id;
    /** Command name */
    std::string cmd_name;
    /** Command value type */
    cmd_param_type_t type;
    /** Command ID */
    control_cmd_t cmd_id;
    /** Command read/write type */
    cmd_rw_t rw;
    /** Number of values the command reads/writes */
    unsigned num_values;
    /** Command info */
    std::string info;
    /** Command visability status */
    bool hidden_cmd;
};

/** @brief Option configuration structure
 * 
 * @note Option names have to be lower case
 */
struct opt_t
{
    /** Long option name */
    std::string long_name;
    /** Short option name */
    std::string short_name;
    /** Option info */
    std::string info;
};

/** @brief Default driver name to use
 * 
 * @note Using I2C by default for now as USB is currently not supported
*/
const std::string default_driver_name = "device_i2c";

/** @brief Convert string to uper case */
std::string to_upper(std::string str);

/** @brief Convert string to lower case */
std::string to_lower(std::string str);

/** @brief Gets device driver name to load */
std::string get_device_lib_name(std::string protocol_name);

/**
 * @brief Open the dynamic library
 * 
 * @param lib_name Name of the library to load (without lib prefix)
 */
dl_handle_t get_dynamic_lib(const std::string lib_name);

/** cmd_t * function pointer type */
using cmd_map_fptr = cmd_t * (*)();

/** uint32_t function pointer type */
using num_cmd_fptr = uint32_t (*)();

/** Function pointer that takes void * and returns Device */
using device_fptr = Device * (*)(void *);

/**
 * @brief Get the function pointer to get_command_map()
 * 
 * @param handle Pointer to the command_map shared object
 */
cmd_map_fptr get_cmd_map_fptr(dl_handle_t handle);

/**
 * @brief Get the function pointer to get_num_commands()
 * 
 * @param handle Pointer to the command_map shared object
 */
num_cmd_fptr get_num_cmd_fptr(dl_handle_t handle);

/**
 * @brief Get the function pointer to make_Dev()
 * 
 * @param handle Pointer to the device shared object
 */
device_fptr get_device_fptr(dl_handle_t handle);

/** @brief Get param type name string */
std::string command_param_type_name(const cmd_param_type_t type);

/** @brief Get read/write type string */
std::string command_rw_type_name(const cmd_rw_t rw);

/** @brief Check if the right number of arguments has been given for the command */
control_ret_t check_num_args(const cmd_t * cmd, const size_t args_left);

/** @brief Convert command line argument from string to cmd_param_t */
cmd_param_t cmd_arg_str_to_val(const cmd_param_type_t type, const char * str);

/** @brief Print cmd_param_t value */
void print_arg(const cmd_param_type_t type, const cmd_param_t val);

/** @brief Exit on control_ret_t error */
void check_cmd_error(std::string cmd_name, std::string rw, control_ret_t ret);

/** @brief Get number of bytes for the particular param type */
size_t get_num_bytes_from_type(const cmd_param_type_t type);

/** @brief Convert single value from bytes to cmd_param_t */
cmd_param_t command_bytes_to_value(const cmd_param_type_t type, const uint8_t * data, unsigned index);

/** @brief Convert single value from cmd_param_t to bytes */
void command_bytes_from_value(const cmd_param_type_t type, uint8_t * data, unsigned index, const cmd_param_t value);

/** @brief Find Levenshtein distance for approximate string matching */
int Levenshtein_distance(const std::string source, const std::string target);

/** @brief Get current terminal width */
size_t get_term_width();

#endif
