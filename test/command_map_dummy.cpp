// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include <stdlib.h>
#include <inttypes.h>
#include "device_control_shared.h"
#include <string>
#include <iostream>
#include <iomanip>

/** @brief Enum for read/write command types */
enum cmd_rw_t {CMD_RO, CMD_WO, CMD_RW};

/**
 * @brief Enum for supported param types
 *
 * @note Add new cmd_param_type's to the end of the list.
 * @note TYPE_CHAR can only be READ ONLY.
 */
enum cmd_param_type_t {TYPE_CHAR, TYPE_UINT8, TYPE_INT32, TYPE_FLOAT, TYPE_UINT32, TYPE_RADIANS};

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
    /** Command visibility status */
    bool hidden_cmd;
};

#define PI_VALUE  3.14159265358979323846f

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

void print_one(const cmd_param_type_t type, const cmd_param_t val)
{
    switch(type)
    {
    case TYPE_CHAR:
        std::cout << static_cast<char>(val.ui8);
        break;
    case TYPE_UINT8:
        std::cout << static_cast<int>(val.ui8) << " ";
        break;
    case TYPE_FLOAT:
        std::cout << std::setprecision(7) << val.f << " ";
        break;
    case TYPE_RADIANS:
        std::cout << std::setprecision(5) << std::fixed << val.f << std::setprecision(2) << std::fixed << " (" << val.f  * 180.0f / PI_VALUE << " deg)" << " ";
        break;
    case TYPE_INT32:
        std::cout << val.i32 << " ";
        break;
    case TYPE_UINT32:
        std::cout << val.ui32 << " ";
        break;
    default:
        std::cerr << "Unsupported parameter type" << std::endl;
        exit(CONTROL_BAD_COMMAND);
    }
}

extern "C"
void super_print_arg(const cmd_t *cmd, cmd_param_t *values)
{
    std::cout << cmd->cmd_name << " ";
    for(unsigned i = 0; i < cmd->num_values; i++)
    {
        print_one(cmd->type, values[i]);
    }
    std::cout << std::endl;
}