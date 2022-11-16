// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef SPECIAL_COMMANDS_H_
#define SPECIAL_COMMANDS_H_

#include "command.hpp"

cmd_t * command_lookup(const std::string str, cmd_t * commands, size_t size);
opt_t * option_lookup(const std::string str);
void print_options_list(void);
void print_command_list(cmd_t * commands, size_t size);
void dump_params(cmd_t * commands, size_t size);

#endif
