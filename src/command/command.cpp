// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "command.hpp"

using namespace std;

Command::Command(Device * _dev, bool _bypass_range, dl_handle_t _handle) :
    device(_dev), bypass_range_check(_bypass_range)
{
    print_args = get_print_args_fptr(_handle);
    check_range = get_check_range_fptr(_handle);
    control_ret_t ret = device->device_init();
    if (ret != CONTROL_SUCCESS)
    {
        cerr << "Could not connect to the device" << endl;
        exit(ret);
    }
}

void Command::init_cmd_info(const string cmd_name)
{
    init_cmd(&cmd, cmd_name);
}

size_t Command::get_num_bytes_from_type()
{
    size_t num_bytes;
    switch(cmd.type)
    {
    case TYPE_CHAR:
    case TYPE_UINT8:
        num_bytes = 1;
        break;
    case TYPE_INT32:
    case TYPE_UINT32:
    case TYPE_FLOAT:
    case TYPE_RADIANS:
        num_bytes = 4;
        break;
    default:
        cerr << "Unsupported parameter type" << endl;
        exit(HOST_APP_ERROR);
    }
    return num_bytes;
}

cmd_param_t Command::cmd_arg_str_to_val(const char * str)
{
    cmd_param_t val;
    try{
        switch(cmd.type)
        {
        case TYPE_CHAR:
            cerr << "TYPE_CHAR commands can only be READ_ONLY" << endl;
            exit(HOST_APP_ERROR);

        case TYPE_UINT8:
        {
            int32_t tmp = stoi(str, nullptr, 0);
            if ((tmp > UINT8_MAX) || (tmp < 0))
            {
                throw out_of_range("");
            }
            val.ui8 = static_cast<uint8_t>(tmp);
            break;
        }
        case TYPE_INT32:
            val.i32 = stoi(str, nullptr, 0);
            break;

        case TYPE_UINT32:
            val.ui32 = stoul(str, nullptr, 0);
            break;

        case TYPE_FLOAT:
        case TYPE_RADIANS:
            val.f = stof(str);
            break;

        default:
            cerr << "Unsupported parameter type" << endl;
            exit(HOST_APP_ERROR);
        }
    }
    catch(const out_of_range & ex)
    {
        static_cast<void>(ex);
        cerr << "Value " << str << " is out of range of " << command_param_type_name(cmd.type) << " type"<< endl;
        exit(HOST_APP_ERROR);
    }
    catch(const invalid_argument & ex)
    {
        static_cast<void>(ex);
        cerr << "Argument " << str << " is invalid" << endl;
        exit(HOST_APP_ERROR);
    }
    return val;
}

control_ret_t Command::command_get(cmd_param_t * values)
{
    control_cmd_t cmd_id = cmd.cmd_id | 0x80; // setting 8th bit for read commands

    size_t data_len = get_num_bytes_from_type()*cmd.num_values + 1; // one extra for the status
    uint8_t * data = new uint8_t[data_len];

    control_ret_t ret = device->device_get(cmd.res_id, cmd_id, data, data_len);
    int read_attempts = 1;

    while(1)
    {
        if(read_attempts == 1000)
        {
            cerr << "Resource could not respond to the " << cmd.cmd_name << " read command."
            << endl << "Check the audio loop is active." << endl;
            exit(HOST_APP_ERROR);
        }
        if(data[0] == CONTROL_SUCCESS)
        {
            for (unsigned i = 0; i < cmd.num_values; i++)
            {
                values[i] = command_bytes_to_value(&data[1], i);
            }
            break;
        }
        else if(data[0] == SERVICER_COMMAND_RETRY)
        {
            ret = device->device_get(cmd.res_id, cmd_id, data, data_len);
            read_attempts++;
        }
        else
        {
            check_cmd_error(cmd.cmd_name, "read", static_cast<control_ret_t>(data[0]));
        }
    }

    delete []data;
    check_cmd_error(cmd.cmd_name, "read", ret);
    return ret;
}

control_ret_t Command::command_set(const cmd_param_t * values)
{
    if(!bypass_range_check)
    {
        check_range(cmd.cmd_name, values);
    }

    size_t data_len = get_num_bytes_from_type() * cmd.num_values;
    uint8_t * data = new uint8_t[data_len];

    for (unsigned i = 0; i < cmd.num_values; i++)
    {
        command_bytes_from_value(data, i, values[i]);
    }

    control_ret_t ret = device->device_set(cmd.res_id, cmd.cmd_id, data, data_len);
    int write_attempts = 1;

    while(1)
    {
        if(write_attempts == 1000)
        {
            cerr << "Resource could not respond to the " << cmd.cmd_name << " write command."
            << endl << "Check the audio loop is active." << endl;
            exit(HOST_APP_ERROR);
        }
        if(ret == CONTROL_SUCCESS)
        {
            break;
        }
        else if(ret == SERVICER_COMMAND_RETRY)
        {
            ret = device->device_set(cmd.res_id, cmd.cmd_id, data, data_len);
            write_attempts++;
        }
        else
        {
            check_cmd_error(cmd.cmd_name, "write", ret);
        }
    }

    delete []data;
    check_cmd_error(cmd.cmd_name, "write", ret);
    return ret;
}

control_ret_t Command::command_get_low_level(uint8_t *data, size_t payload_len)
{
    control_ret_t ret;
    if(payload_len >= 3)
    {
        // Send the byte stream to the device, assuming it has a valid packet format, i.e, res_id, cmd_id, payload_len
        size_t read_len = data[2] + 1;
        uint8_t * read_payload = new uint8_t[read_len];
        ret = device->device_get(data[0], data[1], read_payload, read_len);
        check_cmd_error("TEST_ERROR_HANDLING", "read", static_cast<control_ret_t>(read_payload[0]));
        check_cmd_error("TEST_ERROR_HANDLING", "read", ret);
        delete []read_payload;
    }
    else
    {
        // There's not even 3 bytes in the byte stream. Test by sending whatever's there in the data[] as is to the device
        ret = device->device_get(0, 0, data, payload_len);
        check_cmd_error("TEST_ERROR_HANDLING", "read", static_cast<control_ret_t>(data[0]));
        check_cmd_error("TEST_ERROR_HANDLING", "read", ret);
    }
    return ret;
}

control_ret_t Command::command_set_low_level(uint8_t *data, size_t data_len)
{
    control_ret_t ret;
    if(data_len > 3)
    {
        if(data[2] != data_len-3) // The number of bytes in write payload is less than the data_length in data[2]. Send the bytestream as is to the device
        {
            ret = device->device_set(0, 0, data, data_len);
        }
        else
        {
            // Send the byte stream to the device, assuming it has a valid packet format, i.e, res_id, cmd_id, data_len, followed by payload
            ret = device->device_set(data[0], data[1], &data[3], data[2]);
        }
        check_cmd_error("TEST_ERROR_HANDLING", "write", ret);
    }
    else
    {
        // There's only 3 bytes or less in what's supposedly a write command. Test by sending whatever's there in the data[] as is to the device
        ret = device->device_set(0, 0, data, data_len);
        check_cmd_error("TEST_ERROR_HANDLING", "write", ret);
    }
    return ret;
}

control_ret_t Command::do_command(const string cmd_name, char ** argv, int argc, int arg_indx)
{
    const size_t args_left = argc - arg_indx;
    init_cmd_info(cmd_name);

    control_ret_t ret = check_num_args(&cmd, args_left);
    if (ret != CONTROL_SUCCESS)
    {
        return ret;
    }

    cmd_param_t * cmd_values = new cmd_param_t[cmd.num_values];

    if(args_left == 0) // READ
    {
        ret = command_get(cmd_values);
        print_args(cmd.cmd_name, cmd_values);
    }
    else // WRITE
    {
        for(size_t i = arg_indx; i < arg_indx + args_left; i++)
        {
            cmd_values[i - arg_indx] = cmd_arg_str_to_val(argv[i]);
        }
        ret = command_set(cmd_values);
    }

    delete []cmd_values;

    return ret;
}

cmd_param_t Command::command_bytes_to_value(const uint8_t * data, unsigned index)
{
    cmd_param_t value;
    size_t size_bytes = get_num_bytes_from_type();

    switch(size_bytes)
    {
    case 1:
        memcpy(&value.ui8, data + index * size_bytes, size_bytes);
        break;
    case 4:
        memcpy(&value.i32, data + index * size_bytes, size_bytes);
        break;
    default:
        cerr << "Unsupported parameter type" << endl;
        exit(HOST_APP_ERROR);
    }

    return value;
}

void Command::command_bytes_from_value(uint8_t * data, unsigned index, const cmd_param_t value)
{
    size_t num_bytes = get_num_bytes_from_type();
    switch(num_bytes)
    {
    case 1:
        memcpy(data + index * num_bytes, &value.ui8, num_bytes);
        break;
    case 4:
        memcpy(data + index * num_bytes, &value.i32, num_bytes);
        break;
    default:
        cerr << "Unsupported parameter type" << endl;
        exit(HOST_APP_ERROR);
    }
}

string command_param_type_name(cmd_param_type_t type)
{
    string tstr;

    switch (type)
    {
    case TYPE_CHAR:
        tstr = "char";
        break;

    case TYPE_UINT8:
        tstr = "uint8";
        break;

    case TYPE_INT32:
        tstr = "int32";
        break;

    case TYPE_UINT32:
        tstr = "uint32";
        break;

    case TYPE_FLOAT:
        tstr = "float";
        break;

    case TYPE_RADIANS:
        tstr = "radians";
        break;

    default:
        cerr << "Unsupported parameter type" << endl;
        exit(HOST_APP_ERROR);
    }

    return tstr;
}
