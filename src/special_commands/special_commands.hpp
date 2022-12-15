// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef SPECIAL_COMMANDS_H_
#define SPECIAL_COMMANDS_H_

#include "command.hpp"

void * load_command_map_dll();
cmd_t * command_lookup(const std::string str);
opt_t * option_lookup(const std::string str);
control_ret_t print_help_menu();
control_ret_t print_command_list();
control_ret_t dump_params(Command * command);
control_ret_t execute_cmd_list(Command * command, const char * filename = "commands.txt");
control_ret_t special_cmd_aec_filter(Command * command, bool flag_buffer_get, const char * filename = "aec_filter.bin\0");
control_ret_t special_cmd_nlmodel_buffer(Command * command, bool flag_buffer_get, const char * filename = "nlm_buffer.bin\0");
control_ret_t test_control_interface(Command * command, const char* filename = "test_buffer.bin\0");

#endif
