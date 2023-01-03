// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "utils.hpp"

using namespace std;

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
        exit(CONTROL_BAD_COMMAND);
    }

    return tstr;
}

cmd_param_t cmd_arg_str_to_val(cmd_t * cmd, const char * str)
{
    cmd_param_t val;
    try{
        switch(cmd->type)
        {
        case TYPE_CHAR:
            cerr << "TYPE_CHAR commands can only be READ_ONLY" << endl;
            exit(CONTROL_BAD_COMMAND);
        case TYPE_UINT8:
        {
            int32_t tmp = stoi(str, nullptr, 0);
            if ((tmp >= UINT8_MAX) && (tmp < 0))
            {
                throw out_of_range("");
            }
            val.ui8 = (uint8_t)tmp;
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
            exit(CONTROL_BAD_COMMAND);
        }
    }
    catch(const out_of_range & ex)
    {
        cerr << "Value given is out of range of " << command_param_type_name(cmd->type) << " type"<< endl;
        exit(CONTROL_BAD_COMMAND);
    }
    return val;
}

void print_arg(cmd_t * cmd, cmd_param_t val)
{
    switch(cmd->type)
    {
    case TYPE_CHAR:
        cout << static_cast<char>(val.ui8);
        break;
    case TYPE_UINT8:
        cout << static_cast<int>(val.ui8) << " ";
        break;
    case TYPE_FLOAT:
        cout << val.f << " ";
        break;
    case TYPE_RADIANS:
        cout << val.f << setprecision(2) << fixed << " (" << val.f  * 180.0f / PI_VALUE << " deg)" << setprecision(5) << " ";
        break;
    case TYPE_INT32:
        cout << val.i32 << " ";
        break;
    case TYPE_UINT32:
        cout << val.ui32 << " ";
        break;
    default:
        cerr << "Unsupported parameter type" << endl;
        exit(CONTROL_BAD_COMMAND);
    }
}

size_t get_num_bytes_from_type(cmd_param_type_t type)
{
    size_t num_bytes;
    switch(type)
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
        exit(CONTROL_BAD_COMMAND);
    }
    return num_bytes;
}
