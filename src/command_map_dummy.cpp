// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include <stdlib.h>
#include <inttypes.h>
#include <string>
extern "C"{
#include "device_control_shared.h"
}

#define AEC_RES_ID 0x21
#define PP_RES_ID 0x11

using namespace std;

enum cmd_rw_t {CMD_RO, CMD_WO, CMD_RW};
enum cmd_param_type_t {TYPE_CHAR, TYPE_UINT8, TYPE_INT32, TYPE_FLOAT};
union cmd_param_t {uint8_t ui8; int32_t i32; float f;};

struct cmd_t {
    // Command resource ID
    control_resid_t res_id;
    // Command name
    std::string cmd_name;
    // Command value type
    cmd_param_type_t type;
    // Command ID
    control_cmd_t cmd_id;
    // Command read/write type
    cmd_rw_t rw;
    // Number of values command reads/writes
    unsigned num_values;
    // Command info
    std::string info;
};

cmd_t commands[] = {
        {AEC_RES_ID, "AEC_FAR_EXTGAIN", TYPE_FLOAT,  5, CMD_RW, 1, "Sets/Gets AEC far gain config"},
        {PP_RES_ID,  "PP_AGCDESIREDLVL",TYPE_FLOAT, 11, CMD_RW, 1, "Sets/Gets AGC desired level"  }
};

size_t num_commands = 0;

extern "C"
size_t get_num_commands()
{
    return num_commands;
}

extern "C"
cmd_t* get_command_map()
{
    num_commands = sizeof(commands) / sizeof(cmd_t);
    return commands;
}
