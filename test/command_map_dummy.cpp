// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include <stdlib.h>
#include <inttypes.h>
#include "device_control_shared.h"
#include <string>

/** @brief Enum for read/write command types */
enum cmd_rw_t {CMD_RO, CMD_WO, CMD_RW};

/**
 * @brief Enum for supported param types
 * 
 * @note Add new cmd_param_type's to the end of the list.
 * @note TYPE_CHAR can only be READ ONLY.
 */
enum cmd_param_type_t {TYPE_CHAR, TYPE_UINT8, TYPE_INT32, TYPE_FLOAT, TYPE_UINT32, TYPE_RADIANS};

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

cmd_t commands = {0, "TEST_COMMAND", TYPE_FLOAT, 0, CMD_RW, 60, "test"};
size_t num_commands = 1;

extern "C"
uint32_t get_num_commands()
{
    return num_commands;
}

extern "C"
cmd_t* get_command_map()
{
    return &commands;
}

int i2c_info = 0x2C;    // I2C slave address

extern "C"
int * get_info_i2c()
{
    return &i2c_info;
}
