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
    /** Command visibility status */
    bool hidden_cmd;
};

static cmd_t commands[] = {
                        {0, "CMD_FLOAT",  TYPE_FLOAT,   0, CMD_RW, 20, "This is a test command for testing multiple float reads and writes. Need to keep command descriprion large to test -l option.",   false  },
                        {0, "CMD_INT32",  TYPE_INT32,   1, CMD_RW, 20, "This is a test command for testing multiple int32 reads and writes. Need to keep command descriprion large to test -l option.",   false  },
                        {0, "CMD_UINT32", TYPE_UINT32,  2, CMD_RW, 20, "This is a test command for testing multiple uint32 reads and writes. Need to keep command descriprion large to test -l option.",  false  },
                        {0, "CMD_RADS",   TYPE_RADIANS, 3, CMD_RW, 20, "This is a test command for testing multiple radians reads and writes. Need to keep command descriprion large to test -l option.", false  },
                        {0, "CMD_UINT8",  TYPE_UINT8,   4, CMD_RW, 20, "This is a test command for testing multiple uint8 reads and writes. Need to keep command descriprion large to test -l option.",   false  },
                        {0, "CMD_CHAR",   TYPE_CHAR,    5, CMD_RO, 20, "This is a test command for testing multiple char reads and writes. Need to keep command descriprion large to test -l option.",    false  },
                        {0, "CMD_HIDDEN", TYPE_UINT8,   6, CMD_RW, 20, "This command is suppossed to be hidden and not show up wehn using -l or -d",                                                      true   },
                        {0, "CMD_SMALL",  TYPE_INT32,   7, CMD_RW, 3,  "This is a small command for testing -e option",                                                                                   false  }
};
static size_t num_commands = std::end(commands) - std::begin(commands);

extern "C"
uint32_t get_num_commands()
{
    return num_commands;
}

extern "C"
cmd_t * get_command_map()
{
    return commands;
}

static const int dummy_info = 0x74736574;    // 'test' in ascii

extern "C"
const int * get_info()
{
    return &dummy_info;
}
