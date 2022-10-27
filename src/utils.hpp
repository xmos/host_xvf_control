// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef COMMAND_UTILS_H_
#define COMMAND_UTILS_H_

#include <iostream>
#include <cstring>
#include <stdint.h>
extern "C"{
#include "device_control_shared.h"
}

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

struct opt_t {
    // Long option name
    std::string long_name;
    // Short option name
    std::string short_name;
    // Option info
    std::string info;
    // Number of arguments required by the option
    int num_args;
};

std::string command_param_type_name(cmd_param_type_t type);
std::string command_rw_type_name(cmd_rw_t rw);
control_ret_t check_num_args(cmd_t * cmd, int args_left);
cmd_param_t cmd_arg_str_to_val(cmd_t * cmd, const char * str);
void print_arg(cmd_t * cmd, cmd_param_t val);
unsigned get_num_bytes_from_type(cmd_param_type_t type);
cmd_param_t command_bytes_to_value(cmd_t * cmd, void * data, int index);
void command_bytes_from_value(cmd_t * cmd, void * data, int index, cmd_param_t value);

#endif
