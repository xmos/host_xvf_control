// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "command.hpp"

using namespace std;

Command::Command(Device * _dev) : device(_dev)
{
    control_ret_t ret = device->device_init();
    if (ret != CONTROL_SUCCESS)
    {
        cerr << "Could not connect to the device" << endl;
        exit(ret);
    }
}

control_ret_t Command::command_get(const cmd_t * cmd, cmd_param_t * values)
{
    control_cmd_t cmd_id = cmd->cmd_id | 0x80; // setting 8th bit for read commands

    size_t data_len = get_num_bytes_from_type(cmd->type) * cmd->num_values + 1; // one extra for the status
    uint8_t * data = new uint8_t[data_len];

    control_ret_t ret = device->device_get(cmd->res_id, cmd_id, data, data_len);
    int read_attempts = 1;

    while(1)
    {
        if(read_attempts == 1000)
        {
            cerr << "Could not read from the device" << endl;
            exit(CONTROL_ERROR);
        }
        if(data[0] == CONTROL_SUCCESS)
        {
            for (unsigned i = 0; i < cmd->num_values; i++)
            {
                values[i] = command_bytes_to_value(cmd->type, &data[1], i);
            }
            break;
        }
        else if(data[0] == SERVICER_COMMAND_RETRY)
        {
            ret = device->device_get(cmd->res_id, cmd_id, data, data_len);
            read_attempts++;
        }
        else
        {
            cerr << "Read command " << cmd->cmd_name << " returned control_ret_t error " << data[0] << endl;
            exit(data[0]);
        }
    }

    delete []data;
    check_cmd_error(cmd->cmd_name, "read", ret);
    return ret;
}

control_ret_t Command::command_set(const cmd_t * cmd, const cmd_param_t * values)
{
    size_t data_len = get_num_bytes_from_type(cmd->type) * cmd->num_values;
    uint8_t * data = new uint8_t[data_len];

    for (unsigned i = 0; i < cmd->num_values; i++)
    {
        command_bytes_from_value(cmd->type, data, i, values[i]);
    }

    control_ret_t ret = device->device_set(cmd->res_id, cmd->cmd_id, data, data_len);
    int write_attempts = 1;

    while(1)
    {
        if(write_attempts == 1000)
        {
            cerr << "Could not write to the device" << endl;
            exit(CONTROL_ERROR);
        }
        if(ret == CONTROL_SUCCESS)
        {
            break;
        }
        else if(ret == SERVICER_COMMAND_RETRY)
        {
            ret = device->device_set(cmd->res_id, cmd->cmd_id, data, data_len);
            write_attempts++;
        }
        else
        {
            cerr << "Write command " << cmd->cmd_name << " returned control_ret_t error " << ret << endl;
            exit(ret);
        }
    }

    delete []data;
    check_cmd_error(cmd->cmd_name, "write", ret);
    return ret;
}

control_ret_t Command::do_command(const cmd_t * cmd, char ** argv, int argc, int arg_indx)
{
    const size_t args_left = argc - arg_indx;
    control_ret_t ret = check_num_args(cmd, args_left);
    if (ret != CONTROL_SUCCESS)
    {
        return ret;
    }

    cmd_param_t * cmd_values = new cmd_param_t[cmd->num_values];

    if(args_left == 0) // READ
    {
        ret = command_get(cmd, cmd_values);
        cout << cmd->cmd_name << " ";
        for(unsigned i = 0; i < cmd->num_values; i++)
        {
            print_arg(cmd->type, cmd_values[i]);
        }
        cout << endl;
    }
    else // WRITE
    {
        for(size_t i = arg_indx; i < arg_indx + args_left; i++)
        {
            cmd_values[i - arg_indx] = cmd_arg_str_to_val(cmd->type, argv[i]);
        }
        ret = command_set(cmd, cmd_values);
    }

    delete []cmd_values;

    return ret;
}
