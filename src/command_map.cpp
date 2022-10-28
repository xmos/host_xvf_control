// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include <stdlib.h>
#include <inttypes.h>
#include "command_map.hpp"

#define AEC_RES_ID 0x21
#define PP_RES_ID 0x11

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

cmd_t commands[] = {
        {AEC_RES_ID, "AEC_FAR_EXTGAIN", TYPE_FLOAT,  5, CMD_RW, 1, "Sets/Gets AEC far gain config"},
        {PP_RES_ID,  "PP_AGCDESIREDLVL",TYPE_FLOAT, 11, CMD_RW, 1, "Sets/Gets AGC desired level"  }
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

cmd_t * command_lookup(const string str)
{
    for (int i = 0; i < ARRAY_SIZE(commands); i++)
    {
        cmd_t * cmd = &commands[i];
        if (str == cmd->cmd_name)
        {
            return cmd;
        }
    }

    return nullptr;
}
