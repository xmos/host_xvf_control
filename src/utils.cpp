// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "utils.hpp"
#include <cstring>

using namespace std;

string command_param_type_name(cmd_param_type_t type)
{
    string tstr;

    switch (type)
    { 
    case TYPE_UINT8:
        tstr = "uint8";
        break;
    
    case TYPE_INT32:
        tstr = "int32";
        break;

    case TYPE_FLOAT:
        tstr = "float";
        break;

    default:
        cout << "Unsupported parameter type." << endl;
        exit(CONTROL_BAD_COMMAND);
    }

    return tstr;
}

string command_rw_type_name(cmd_rw_t rw)
{
    string tstr;

    switch(rw){
    case CMD_RO:
        tstr = "READ ONLY";
        break;

    case CMD_WO:
        tstr = "WRITE_ONLY";
        break;

    case CMD_RW:
        tstr = "READ/WRITE";
        break;

    default:
        tstr = "unknown";
        break;
    }

    return tstr;
}

control_ret_t check_num_args(cmd_t * cmd, int args_left)
{
    if((cmd->rw == CMD_RO) && (args_left != 0))
    {
        cout << "Error: Command: " << cmd->cmd_name << " is read-only, so it does not require any arguments." << endl;
        exit(CONTROL_DATA_LENGTH_ERROR);
    }
    else if ((cmd->rw == CMD_WO) && (args_left != cmd->num_values))
    {
        cout << "Error: Command: " << cmd->cmd_name << " is write-only and"
        << " expects " << cmd->num_values << " argument(s), " << endl
        << args_left << " are given." << endl;
        exit(CONTROL_DATA_LENGTH_ERROR);
    }
    else if ((cmd->rw == CMD_RW) && (args_left != 0) && (args_left != cmd->num_values))
    {
        cout << "Error: Command: " << cmd->cmd_name << " is a read/write command." << endl
        << "If you want to read do not give any arguments to this command." << endl
        << "If you want to write give " << cmd->num_values << " argument(s) to this command." << endl;
        exit(CONTROL_DATA_LENGTH_ERROR);
    }
    return CONTROL_SUCCESS;
}

cmd_param_t cmd_arg_str_to_val(cmd_t * cmd, const char * str)
{
    cmd_param_t val;
    try{
        switch(cmd->type)
        {
        case TYPE_UINT8:
        {
            int32_t tmp = stoi(str);
            if ((tmp >= UINT8_MAX) && (tmp < 0))
            {
                throw out_of_range("");
            }
            val.ui8 = (uint8_t)tmp;
        }
        case TYPE_INT32:
            val.i32 = stoi(str);
            break;
        case TYPE_FLOAT:
            val.f = stof(str);
            break;
        }
    }
    catch(const out_of_range & ex)
    {
        cout << "Value given is out of range of " << command_param_type_name(cmd->type) << " type."<< endl;
        exit(CONTROL_BAD_COMMAND);
    }
    return val;
}

void print_arg(cmd_t * cmd, cmd_param_t val)
{
    switch(cmd->type)
    {
    case TYPE_CHAR:
        cout << to_string(val.ui8);
        break;
    case TYPE_UINT8:
        cout << val.ui8;
        break;
    case TYPE_FLOAT:
        cout << val.f;
        break;
    case TYPE_INT32:
        cout << val.i32;
        break;
    }
}

unsigned get_num_bytes_from_type(cmd_param_type_t type)
{
    unsigned num_bytes;
    switch(type)
    {
    case TYPE_UINT8:
        num_bytes = 1;
        break;
    case TYPE_INT32:
    case TYPE_FLOAT:
        num_bytes = 4;
        break;
    default:
        std::cout << "Unsupported parameter type." << std::endl;
        exit(CONTROL_BAD_COMMAND);
    }
    return num_bytes;
}

cmd_param_t command_bytes_to_value(cmd_t * cmd, void * data, int index)
{
    cmd_param_t value;
    unsigned size_bytes = get_num_bytes_from_type(cmd->type);

    switch(size_bytes)
    {
    case 1:
        memcpy(&value.ui8, data + index * size_bytes, size_bytes);
        break;
    case 4:
        memcpy(&value.i32, data + index * size_bytes, size_bytes);
        break;
    }

    return value;
}

void command_bytes_from_value(cmd_t * cmd, void * data, int index, cmd_param_t value)
{
    unsigned num_bytes = get_num_bytes_from_type(cmd->type);
    switch(num_bytes)
    {
    case 1:
        memcpy(data + index * num_bytes, &value.ui8, num_bytes);
        break;
    case 4:
        memcpy(data + index * num_bytes, &value.i32, num_bytes);
        break;
    }
}

