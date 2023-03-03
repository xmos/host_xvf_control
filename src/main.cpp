// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "special_commands.hpp"

using namespace std;

int main(int argc, char ** argv)
{
    if(argc == 1)
    {
        cout << "Use --help to get the list of options for this application." << endl
        << "Or use --list-commands to print the list of commands and their info." << endl;
        return 0;
    }
    
    int cmd_indx = 1;
    dl_handle_t cmd_map_handle = load_command_map_dll();
    string lib_name = get_device_lib_name(&argc, argv);
    bool bypass_range_check = get_bypass_range_check(&argc, argv);

    cmd_t * cmd = nullptr;
    opt_t * opt = nullptr;
    string next_cmd = argv[cmd_indx];

    if(next_cmd[0] == '-')
    {
        opt = option_lookup(next_cmd);
        if (opt->long_name == "--help")
        {
            return print_help_menu();
        }
        else if (opt->long_name == "--version")
        {
            cout << current_host_app_version << endl;
            return 0;
        }
        else if (opt->long_name == "--list-commands")
        {
            return print_command_list();
        }
    }

    dl_handle_t device_handle = get_dynamic_lib(lib_name);
    print_args_fptr print_args = get_print_args_fptr(cmd_map_handle);
    device_fptr make_dev = get_device_fptr(device_handle);
    auto device = make_dev(cmd_map_handle);

    Command command(device, bypass_range_check, print_args);

    int arg_indx = cmd_indx + 1;
    next_cmd = argv[cmd_indx];

    if(next_cmd[0] == '-')
    {
        opt = option_lookup(next_cmd);
    }
    else
    {
        cmd = command_lookup(next_cmd);
    }

    if (cmd != nullptr)
    {
        return command.do_command(cmd, argv, argc, arg_indx);
    }
    else
    {
        if(opt->long_name == "--dump-params")
        {
            return dump_params(&command);
        }
        if(opt->long_name == "--execute-command-list")
        {
            if(arg_indx >= argc)
            {
                return execute_cmd_list(&command);
            }
            else
            {
                return execute_cmd_list(&command, argv[arg_indx]);
            }
        }
        if(opt->long_name == "--get-aec-filter")
        {
            if(arg_indx >= argc)
            {
                return special_cmd_aec_filter(&command, true);
            }
            else
            {
                return special_cmd_aec_filter(&command, true, argv[arg_indx]);
            } 
        }
        if(opt->long_name == "--set-aec-filter")
        {
            if(arg_indx >= argc)
            {
                return special_cmd_aec_filter(&command, false);
            }
            else
            {
                return special_cmd_aec_filter(&command, false, argv[arg_indx]);
            }
        }        
        if(opt->long_name == "--get-nlmodel-buffer")
        {
            if(arg_indx >= argc)
            {
                return special_cmd_nlmodel_buffer(&command, true);
            }
            else
            {
                return special_cmd_nlmodel_buffer(&command, true, argv[arg_indx]);
            }
        }
        if(opt->long_name == "--set-nlmodel-buffer")
        {
            if(arg_indx >= argc)
            {
                return special_cmd_nlmodel_buffer(&command, false);
            }
            else
            {
                return special_cmd_nlmodel_buffer(&command, false, argv[arg_indx]);
            }
        }
        if(opt->long_name == "--test-control-interface")
        {
            return test_control_interface(&command, argv[arg_indx]);
        }
        if(opt->long_name == "--test-bytestream")
        {
            return test_bytestream(&command, argv[arg_indx]);
        }
    }
    // Program should NEVER get to this point
    cout << "Host application behaved unexpectedly, please report this issue" << endl;
    return HOST_APP_ERROR;
}
