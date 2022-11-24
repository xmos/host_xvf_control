// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "command.hpp"

using namespace std;

Command::Command(Device * _dev) : device(_dev) {}

void Command::init_device()
{
    control_ret_t ret = device->device_init();
    if (ret != CONTROL_SUCCESS)
    {
        cout << "Could not connect to the device" << endl;
        exit(ret);
    }
}

control_ret_t Command::command_get(cmd_t * cmd, cmd_param_t * values, int num_values)
{
    int read_attempts = 0;

    size_t data_len = sizeof(cmd_param_t) * cmd->num_values + 1; // one extra for the status
    uint8_t * data = new uint8_t[data_len];

    control_ret_t ret = device->device_get(cmd->res_id, cmd->cmd_id, data, data_len);
    read_attempts++;

    while(1){
        if (ret != CONTROL_SUCCESS){break;}
        else{
            if(read_attempts == 1000){cout << "Read is taking a while.." << endl;}
            if(data[0] != CONTROL_SUCCESS)
            {
                ret = device->device_get(cmd->res_id, cmd->cmd_id, data, data_len);
                read_attempts++;
            }
            else
            {
                for (int i = 0; i < cmd->num_values; i++)
                {
                    values[i] = command_bytes_to_value(cmd, &data[1], i);
                }
                break;
            }
        }
    }
    delete []data;

    return ret;
}

control_ret_t Command::command_set(cmd_t * cmd, const cmd_param_t * values, int num_values)
{
    size_t data_len = sizeof(cmd_param_t) * cmd->num_values;
    uint8_t * data = new uint8_t[data_len];

    for (int i = 0; i < cmd->num_values; i++) {
        command_bytes_from_value(cmd, data, i, values[i]);
    }

    control_ret_t ret = device->device_set(cmd->res_id, cmd->cmd_id, data, data_len);

    delete []data;

    return ret;
}

control_ret_t Command::do_command(cmd_t * cmd, char ** argv, int args_left)
{
    init_device();
    
    control_ret_t ret = check_num_args(cmd, args_left);
    if (ret != CONTROL_SUCCESS)
    {
        return ret;
    }

    cmd_param_t * cmd_values = new cmd_param_t[cmd->num_values];

    if(args_left == 0) // READ
    {
        cmd->cmd_id |= 0x80; // setting 8th bit for read commands
        ret = command_get(cmd, cmd_values, cmd->num_values);
        cout << cmd->cmd_name;
        for(int i = 0; i < cmd->num_values; i++)
        {
            cout << " ";
            print_arg(cmd, cmd_values[i]);
        }
        cout << endl;
    }
    else // WRITE
    {
        for(int i = 2; i < 2 + args_left; i++)
        {
            cmd_values[i - 2] = cmd_arg_str_to_val(cmd, argv[i]);
        }
        ret = command_set(cmd, cmd_values, cmd->num_values);
    }

    delete []cmd_values;

    return ret;
}
