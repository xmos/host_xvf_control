// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "special_commands.hpp"

using namespace std;

int main(int argc, char ** argv)
{
    control_ret_t ret = CONTROL_ERROR;

    int cmd_indx = 1;
    int args_left = argc - cmd_indx - 1;

    cmd_t * cmd = command_lookup(argv[cmd_indx]);
    opt_t * opt = option_lookup(argv[cmd_indx]);

    if ((cmd == nullptr) && (opt == nullptr))
    {
        cout << "Command " << argv[cmd_indx] << " does not exit." << endl;
        return CONTROL_BAD_COMMAND;
    }

    if (cmd != nullptr)
    {
        Command command;
        command.do_command(cmd, argv, args_left);
    }
    else
    {
        if(opt->long_name == "--help")
        {
            print_command_list();
        }
        if(opt->long_name == "--list-commands")
        {
            print_options_list();
        }
    }

    return ret;
}
