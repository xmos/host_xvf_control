// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "special_commands.hpp"
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

using namespace std;

opt_t options[] = {
            {"--help",                 "-h", "print this options menu",          0},
            {"--list-commands",        "-l", "print the list of commands",       0},
            {"--vendor-id",            "-v", "set USB Vendor ID",                1},
            {"--product-id",           "-p", "set USB Product ID",               1},
            {"--dump-params",          "-d", "print all the parameters",         0},
            {"--skip-version-check",   "-s", "skip version check",               0},
            {"--execute-command-list", "-e", "execute commands from .txt file",  1}
};

opt_t * option_lookup(const string str)
{
    for(int i = 0; i < ARRAY_SIZE(options); i++)
    {
        opt_t * opt = &options[i];
        if ((str == opt->long_name) || (str == opt->short_name))
        {
            return opt;
        }
    }
    return nullptr;
}

cmd_t * command_lookup(const string str, cmd_t * commands, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        cmd_t * cmd = &commands[i];
        if (str == cmd->cmd_name)
        {
            return cmd;
        }
    }
    return nullptr;
}

void print_options_list(void)
{
    for(opt_t opt : options)
    {
        cout << "Use " << opt.short_name <<  " or " << opt.long_name
        << " to " << opt.info << "." << endl;
    }
}

void print_command_list(cmd_t * commands, size_t size)
{
    for(size_t i = 0; i < size; i ++)
    {
        cmd_t * cmd = &commands[i];
        cout << cmd->cmd_name << " is " << command_rw_type_name(cmd->rw)
        << " and requires/returns " << cmd->num_values
        << " values of type " << command_param_type_name(cmd->type)
        << ". "<< cmd->info << "." << endl;
    }
}

void dump_params(cmd_t * commands, size_t size)
{
    Command command;
    for(size_t i = 0; i < size; i ++)
    {
        command.do_command(&commands[i], nullptr, 0);
    }
}
