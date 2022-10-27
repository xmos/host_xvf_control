// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef COMMAND_CLASS_H_
#define COMMAND_CLASS_H_

#include "device.hpp"
#include "utils.hpp"

class Command
{
    private:
        Device device;
        void init_device();
        control_ret_t command_get(cmd_t * cmd, cmd_param_t * values, int num_values);
        control_ret_t command_set(cmd_t * cmd, const cmd_param_t * values, int num_values);

    public:
        control_ret_t do_command(cmd_t * cmd, char ** argv, int args_left);
};

#endif
