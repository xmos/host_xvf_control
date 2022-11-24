// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef SPECIAL_COMMANDS_H_
#define SPECIAL_COMMANDS_H_

#include "command.hpp"

void load_command_map_dll();
cmd_t * command_lookup(const std::string str);
opt_t * option_lookup(const std::string str);
control_ret_t print_options_list();
control_ret_t print_command_list();
control_ret_t dump_params(Command * command);
control_ret_t get_aec_filter(Command * command, const char * filename);
control_ret_t special_cmd_nlmodel_buffer(Command * command, const char * filename, bool flag_buffer_get);

#endif
