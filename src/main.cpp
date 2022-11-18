// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "special_commands.hpp"
#include "dlfcn.h"

using namespace std;

cmd_t * commands;
size_t num_commands;

int main(int argc, char ** argv)
{
    if(argc == 1)
    {
        cout << "Use --help to get the list of options for this application." << endl
        << "Or use --list-commands to print the list of commands and their info." << endl;
        return 0;
    }

    string dyn_lib_path = get_dynamic_lib_path();
    void * sofile = dlopen(dyn_lib_path.c_str(), RTLD_NOW);
    if(!sofile)
    {
        printf("%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    cmd_t* (*get_command_map)();
    get_command_map = (cmd_t* (*)())dlsym(sofile, "get_command_map");

    uint32_t (*get_num_commands)();
    get_num_commands = (uint32_t (*)())dlsym(sofile, "get_num_commands");

    commands = get_command_map();
    num_commands = get_num_commands();
    
    control_ret_t ret = CONTROL_ERROR;

    int cmd_indx = 1;
    int args_left = argc - cmd_indx - 1;

    cmd_t * cmd = command_lookup(argv[cmd_indx], commands, num_commands);
    opt_t * opt = option_lookup(argv[cmd_indx]);

    if ((cmd == nullptr) && (opt == nullptr))
    {
        cout << "Command " << argv[cmd_indx] << " does not exit." << endl;
        return CONTROL_BAD_COMMAND;
    }

    if (cmd != nullptr)
    {
        Command command;
        ret = command.do_command(cmd, argv, args_left);
    }
    else
    {
        if(opt->long_name == "--help")
        {
            ret = print_options_list();
        }
        if(opt->long_name == "--list-commands")
        {
            ret = print_command_list(commands, num_commands);
        }
        if(opt->long_name == "--dump-params")
        {
            ret = dump_params(commands, num_commands);
        }
    }

    return ret;
}
