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

static cmd_t commands[] = {
                        {0, "CMD_FLOAT", TYPE_FLOAT, 0, CMD_RW, 20, "test float"},
                        {0, "CMD_UINT8", TYPE_UINT8, 1, CMD_RW, 20, "test uint8"}
};
static size_t num_commands = std::end(commands) - std::begin(commands);

extern "C"
#if defined(_WIN32)
__declspec(dllexport)
#endif
uint32_t get_num_commands()
{
    return num_commands;
}

extern "C"
#if defined(_WIN32)
__declspec(dllexport)
#endif
cmd_t * get_command_map()
{
    return commands;
}

static const int dummy_info = 0x74736574;    // 'test' in ascii

extern "C"
#if defined(_WIN32)
__declspec(dllexport)
#endif
const int * get_info()
{
    return &dummy_info;
}
