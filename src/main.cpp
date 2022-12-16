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
        return CONTROL_SUCCESS;
    }

    int cmd_indx = 1;
    void * cmd_map_handle = load_command_map_dll();
    cmd_t * cmd = nullptr;
    opt_t * opt = nullptr;
    string next_arg = argv[cmd_indx];
    // Using I2C by default for now as USB is not supported
    string lib_name = "libdevice_rpi_i2c";
    if(next_arg[0] != '-')
    {
        cmd = command_lookup(next_arg);
    }
    else
    {
        opt = option_lookup(next_arg);
        if (opt->long_name == "--help")
        {
            return print_help_menu();
        }
        else if (opt->long_name == "--list-commands")
        {
            return print_command_list();
        }
        else if (opt->long_name == "--use")
        {
            string protocol_name = argv[cmd_indx + 1];
            if (to_upper(protocol_name) == "I2C")
            {
                lib_name = "libdevice_rpi_i2c";
            }
            else if (to_upper(protocol_name) == "SPI")
            {
                lib_name = "libdevice_rpi_spi";
            }
            else
            {
                // Using I2C by default for now as USB is not supported
                clog << "Did not find " << to_upper(protocol_name) << "in supported protocols"
                << endl << "Will use I2C by default" << endl;
            }

            cmd_indx += 2; // fetched --use something
        }
    }

    string device_lib_path = get_dynamic_lib_path(lib_name);
    factory fact(device_lib_path.c_str());
    auto device = fact.make_dev(cmd_map_handle);
    Command command(device.get());

    int arg_indx = cmd_indx + 1;
    int args_left = argc - cmd_indx - 1;
    next_arg = argv[cmd_indx];

    if (cmd != nullptr)
    {
        return command.do_command(cmd, argv, args_left, arg_indx);
    }

    if(next_arg[0] == '-')
    {
        opt = option_lookup(next_arg);
    }
    else
    {
        cmd = command_lookup(next_arg);
    }

    if (cmd != nullptr)
    {
        return command.do_command(cmd, argv, args_left, arg_indx);
    }
    else
    {
        if(opt->long_name == "--help")
        {
            return print_help_menu();
        }
        if(opt->long_name == "--list-commands")
        {
            return print_command_list();
        }
        if(opt->long_name == "--use")
        {
            cerr << "Incorrect use of the host application. Use --help to see the usage.";
            return CONTROL_ERROR;
        }
        if(opt->long_name == "--dump-params")
        {
            return dump_params(&command);
        }
        if(opt->long_name == "--execute-command-list")
        {
            if(argv[arg_indx] == NULL)
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
            if(argv[arg_indx] == NULL)
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
            if(argv[arg_indx] == NULL)
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
            if(argv[arg_indx] == NULL)
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
            if(argv[arg_indx] == NULL)
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
    }
    // Program should NEVER get to this point
    cout << "Host application behaved unexpectedly, please report this issue" << endl;
    return CONTROL_ERROR;
}
