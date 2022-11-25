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
    
    opt_t * first_opt = option_lookup(argv[1]);

    if (first_opt == nullptr)
    {
        cout << "First argument to this application should be -u or -h or -l" << endl;
        return CONTROL_ERROR;
    }
    else if (first_opt->long_name == "--help")
    {
        return print_help_menu();
    }
    else if (first_opt->long_name == "--list_commands")
    {
        return print_command_list();
    }
    else if (first_opt->long_name != "--use")
    {
        cout << "First argument to this application should be -u or -h or -l" << endl;
        return CONTROL_ERROR;
    }

    string name = argv[2];
    char lib_name[20];

    if ((name == "i2c") || (name == "I2C"))
    {
        sprintf(lib_name, "%s", "./libdevice_i2c.so\0");
    }
    if ((name == "spi") || (name == "SPI"))
    {
        sprintf(lib_name, "%s", "./libdevice_spi.so\0");
    }
    cout << "libname : " << lib_name << endl;
    int cmd_indx = 3;
    int args_left = argc - cmd_indx - 1;

    cmd_t * cmd = command_lookup(argv[cmd_indx]);
    opt_t * opt = option_lookup(argv[cmd_indx]);

    if ((cmd == nullptr) && (opt == nullptr))
    {
        cout << "Command " << argv[cmd_indx] << " does not exit." << endl;
        return CONTROL_BAD_COMMAND;
    }

    factory fact(lib_name);
    auto device = fact.make_dev();
    Command command(device.get());

    if (cmd != nullptr)
    {
        ret = command.do_command(cmd, argv, args_left);
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
        if(opt->long_name == "--get-aec-filter")
        {
            if(argv[cmd_indx + 1] == NULL)
            {
                ret = get_aec_filter(&command);
            }
            else
            {
                ret = get_aec_filter(&command, argv[cmd_indx + 1]);
            } 
        }
        if(opt->long_name == "--get-nlmodel-buffer")
        {
            if(argv[cmd_indx + 1] == NULL)
            {
                ret = special_cmd_nlmodel_buffer(&command, true);
            }
            else
            {
                ret = special_cmd_nlmodel_buffer(&command, true, argv[cmd_indx + 1]);
            }
        }
        if(opt->long_name == "--set-nlmodel-buffer")
        {
            if(argv[cmd_indx + 1] == NULL)
            {
                ret = special_cmd_nlmodel_buffer(&command, false);
            }
            else
            {
                ret = special_cmd_nlmodel_buffer(&command, false, argv[cmd_indx + 1]);
            }
        }
    }

    return ret;
}
