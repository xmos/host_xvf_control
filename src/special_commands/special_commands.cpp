// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "special_commands.hpp"

using namespace std;

void print_options_list(void)
{
    for(opt_t opt : options)
    {
        cout << "Use " << opt.short_name <<  " or " << opt.long_name
        << " to " << opt.info << "." << endl;
    }
}

void print_command_list(void)
{
    for(cmd_t cmd : commands)
    {
        cout << cmd.cmd_name << " is " << command_rw_type_name(cmd.rw)
        << " and requires/returns " << cmd.num_values
        << " values of type " << command_param_type_name(cmd.type)
        << ". "<< cmd.info << "." << endl;
    }
}
