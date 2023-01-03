// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef COMMAND_UTILS_H_
#define COMMAND_UTILS_H_

#include <iostream>
#include <cstring>
#include <stdint.h>
extern "C"
#include "device_control_shared.h"

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
    /** Second line of info */
    std::string more_info;
};

/** @brief Convert string to uper case */
std::string to_upper(std::string str);

/** @brief Convert string to lower case */
std::string to_lower(std::string str);

/**
 * @brief Get the dynamic library path.
 * 
 * Will get the path to the running application and use lib_name
 * to get the full path to dynamic library. Works for Linux, Mac and Windows.
 * 
 * @param lib_name  Name of the library to load (without lib prefix)
 */
std::string get_dynamic_lib_path(std::string lib_name);

/** @brief Get param type name string */
std::string command_param_type_name(cmd_param_type_t type);

/** @brief Get read/write type string */
std::string command_rw_type_name(cmd_rw_t rw);

/** @brief Check if the right number of arguments has been given for the command */
control_ret_t check_num_args(cmd_t * cmd, int args_left);

/** @brief Convert command line argument from string to cmd_param_t */
cmd_param_t cmd_arg_str_to_val(cmd_t * cmd, const char * str);

/** @brief Print cmd_param_t value */
void print_arg(cmd_t * cmd, cmd_param_t val);

/** @brief Exit on control_ret_t error */
void check_cmd_error(std::string cmd_name, std::string rw, control_ret_t ret);

/** @brief Get number of bytes for the particular param type */
unsigned get_num_bytes_from_type(cmd_param_type_t type);

/** @brief Convert single value from bytes to cmd_param_t */
cmd_param_t command_bytes_to_value(cmd_t * cmd, uint8_t * data, int index);

/** @brief Convert single value from cmd_param_t to bytes */
void command_bytes_from_value(cmd_t * cmd, uint8_t * data, int index, cmd_param_t value);

/** @brief Find Levenshtein distance for approximate string matching */
int levDistance(const std::string source, const std::string target);

#endif
