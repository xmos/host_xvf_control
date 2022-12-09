// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "special_commands.hpp"
#include "factory.hpp"

using namespace std;

int main(int argc, char ** argv)
{
    if(argc == 1)
    {
        cout << "Use --help to get the list of options for this application." << endl
        << "Or use --list-commands to print the list of commands and their info." << endl;
        return 0;
    }

    control_ret_t ret = CONTROL_ERROR;
    void * cmd_map_handle = load_command_map_dll();
    opt_t * first_opt = option_lookup(argv[1]);

    if (first_opt->long_name == "--help")
    {
        return print_help_menu();
    }
    else if (first_opt->long_name == "--list-commands")
    {
        return print_command_list();
    }
    else if (first_opt->long_name != "--use")
    {
        cout << "First argument to this application should be -u or -h or -l" << endl;
        return CONTROL_ERROR;
    }

    string name = argv[2];
    string lib_name;

    if (to_upper(name) == "I2C")
    {
        lib_name = "/libdevice_i2c_rpi";
    }
    if (to_upper(name) == "SPI")
    {
        lib_name = "/libdevice_spi_rpi";
    }

    int cmd_indx = 3;
    int arg_indx = cmd_indx + 1;
    int args_left = argc - cmd_indx - 1;
    string next_arg = argv[cmd_indx];
    cmd_t * cmd = nullptr;
    opt_t * opt = nullptr;

    if(next_arg[0] == '-')
    {
        opt = option_lookup(next_arg);
    }
    else
    {
        cmd = command_lookup(next_arg);
    }

    string device_lib_path = get_dynamic_lib_path(lib_name);
    factory fact(device_lib_path.c_str());
    auto device = fact.make_dev(cmd_map_handle);
    Command command(device.get());

    if (cmd != nullptr)
    {
        ret = command.do_command(cmd, argv, args_left, arg_indx);
    }
    else
    {
        if(opt->long_name == "--help")
        {
            ret = print_help_menu();
        }
        if(opt->long_name == "--list-commands")
        {
            ret = print_command_list();
        }
        if(opt->long_name == "--dump-params")
        {
            ret = dump_params(&command);
        }
        if(opt->long_name == "--execute-command-list")
        {
            if(argv[arg_indx] == NULL)
            {
                ret = execute_cmd_list(&command);
            }
            else
            {
                ret = execute_cmd_list(&command, argv[arg_indx]);
            }
        }
        if(opt->long_name == "--get-aec-filter")
        {
            if(argv[arg_indx] == NULL)
            {
                ret = special_cmd_aec_filter(&command, true);
            }
            else
            {
                ret = special_cmd_aec_filter(&command, true, argv[arg_indx]);
            } 
        }
        if(opt->long_name == "--set-aec-filter")
        {
            if(argv[arg_indx] == NULL)
            {
                ret = special_cmd_aec_filter(&command, false);
            }
            else
            {
                ret = special_cmd_aec_filter(&command, false, argv[arg_indx]);
            }
        }        
        if(opt->long_name == "--get-nlmodel-buffer")
        {
            if(argv[arg_indx] == NULL)
            {
                ret = special_cmd_nlmodel_buffer(&command, true);
            }
            else
            {
                ret = special_cmd_nlmodel_buffer(&command, true, argv[arg_indx]);
            }
        }
        if(opt->long_name == "--set-nlmodel-buffer")
        {
            if(argv[arg_indx] == NULL)
            {
                ret = special_cmd_nlmodel_buffer(&command, false);
            }
            else
            {
                ret = special_cmd_nlmodel_buffer(&command, false, argv[arg_indx]);
            }
        }
        if(opt->long_name == "--test-control-interface")
        {
            ret = test_control_interface(&command, argv[arg_indx]);
        }
    }

    return ret;
}
