// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef SPECIAL_COMMANDS_H_
#define SPECIAL_COMMANDS_H_

#include "command.hpp"

cmd_t * command_lookup(const std::string str, cmd_t * commands, size_t size);
opt_t * option_lookup(const std::string str);
control_ret_t print_options_list(void);
control_ret_t print_command_list(cmd_t * commands, size_t size);
control_ret_t dump_params(cmd_t * commands, size_t size);

void get_or_set_full_buffer(cmd_param_t *buffer, int32_t buffer_length, cmd_t *start_coeff_index_cmd, cmd_t *filter_cmd, int flag_buffer_get);
void get_one_filter(int32_t mic_index, int32_t far_index, std::string filename, uint32_t buffer_length, cmd_t * commands, size_t num_commands);
control_ret_t get_aec_filter(const char *filename, cmd_t * commands, size_t size);
void special_cmd_nlmodel_buffer(const char* filename, int32_t flag_buffer_get, cmd_t * commands, size_t num_commands);

#endif
