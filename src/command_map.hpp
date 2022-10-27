// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "utils.hpp"

#define NUM_OPTIONS_SUPPORTED 7
#define NUM_COMMANDS_SUPPORTED 2

extern opt_t options[NUM_OPTIONS_SUPPORTED];
extern cmd_t commands[NUM_COMMANDS_SUPPORTED];

cmd_t * command_lookup(const std::string str);
opt_t * option_lookup(const std::string str);

#endif
