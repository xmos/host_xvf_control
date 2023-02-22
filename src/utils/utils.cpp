// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "utils.hpp"
#include <vector>
#include "control_ret_str_map.h"

using namespace std;

string to_upper(string str)
{
    for(unsigned i = 0; i < str.length(); i++)
    {
        str[i] = toupper(str[i]);
    }
    return str;
}

string to_lower(string str)
{
    for(unsigned i = 0; i < str.length(); i++)
    {
        str[i] = tolower(str[i]);
    }
    return str;
}

string get_device_lib_name(string protocol_name)
{
    string lib_name = default_driver_name;
    if (to_upper(protocol_name) == "I2C")
    {
        lib_name = "device_i2c";
    }
    else if (to_upper(protocol_name) == "SPI")
    {
        lib_name = "device_spi";
    }
    else
    {
        // Using I2C by default for now as USB is currently not supported
        cout << "Could not find " << to_upper(protocol_name) << " in supported protocols"
        << endl << "Will use I2C by default" << endl;
    }
    return lib_name;
}

void check_cmd_error(string cmd_name, string rw, control_ret_t ret)
{
    rw[0] = toupper(rw[0]);
    if(ret != CONTROL_SUCCESS)
    {
        cerr << rw << " command " << cmd_name << " returned control_ret_t error " << static_cast<int>(ret) << ", " << control_ret_str_map[ret] << endl;
        exit(ret);
    }
}

string command_rw_type_name(const cmd_rw_t rw)
{
    string tstr;

    switch(rw){
    case CMD_RO:
        tstr = "READ ONLY";
        break;

    case CMD_WO:
        tstr = "WRITE ONLY";
        break;

    case CMD_RW:
        tstr = "READ/WRITE";
        break;

    default:
        cerr << "Unsupported read/write type" << endl;
        exit(HOST_APP_ERROR);
    }

    return tstr;
}

control_ret_t check_num_args(const cmd_t * cmd, const size_t args_left)
{
    if((cmd->rw == CMD_RO) && (args_left != 0))
    {
        cerr << "Command: " << cmd->cmd_name << " is read-only, so it does not require any arguments." << endl;
        exit(HOST_APP_ERROR);
    }
    else if ((cmd->rw == CMD_WO) && (args_left != cmd->num_values))
    {
        cerr << "Command: " << cmd->cmd_name << " is write-only and"
        << " expects " << cmd->num_values << " argument(s), " << endl
        << args_left << " are given." << endl;
        exit(HOST_APP_ERROR);
    }
    else if ((cmd->rw == CMD_RW) && (args_left != 0) && (args_left != cmd->num_values))
    {
        cerr << "Command: " << cmd->cmd_name << " is a read/write command." << endl
        << "If you want to read do not give any arguments to this command." << endl
        << "If you want to write give " << cmd->num_values << " argument(s) to this command, "
        << args_left << " are given." << endl;
        exit(HOST_APP_ERROR);
    }
    return CONTROL_SUCCESS;
}

cmd_param_t command_bytes_to_value(const cmd_param_type_t type, const uint8_t * data, unsigned index)
{
    cmd_param_t value;
    size_t size_bytes = get_num_bytes_from_type(type);

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

void command_bytes_from_value(const cmd_param_type_t type, uint8_t * data, unsigned index, const cmd_param_t value)
{
    size_t num_bytes = get_num_bytes_from_type(type);
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


// Taken from:
// https://www.talkativeman.com/levenshtein-distance-algorithm-string-comparison/
int Levenshtein_distance(const string source, const string target)
{

    const int n = source.length();
    const int m = target.length();
    if (n == 0)
    {
        return m;
    }
    if (m == 0)
    {
        return n;
    }

    typedef vector<vector<int>> Tmatrix; 

    Tmatrix matrix(n + 1);

    // Size the vectors in the 2.nd dimension. Unfortunately C++ doesn't
    // allow for allocation on declaration of 2.nd dimension of vec of vec

    for (int i = 0; i <= n; i++)
    {
        matrix[i].resize(m + 1);
    }

    for (int i = 0; i <= n; i++)
    {
        matrix[i][0] = i;
    }

    for (int j = 0; j <= m; j++)
    {
        matrix[0][j] = j;
    }

    for (int i = 1; i <= n; i++)
    {

        const char s_i = source[i - 1];

        for (int j = 1; j <= m; j++)
        {

            const char t_j = target[j - 1];

            int cost;
            if (s_i == t_j)
            {
                cost = 0;
            }
            else
            {
                cost = 1;
            }

            const int above = matrix[i - 1][j];
            const int left = matrix[i][j - 1];
            const int diag = matrix[i - 1][j - 1];
            int cell = min( above + 1, min(left + 1, diag + cost));

            // Cover transposition, in addition to deletion,
            // insertion and substitution. This step is taken from:
            // Berghel, Hal ; Roach, David : "An Extension of Ukkonen's 
            // Enhanced Dynamic Programming ASM Algorithm"
            // (http://www.acm.org/~hlb/publications/asm/asm.html)

            if (i > 2 && j > 2)
            {
                int trans = matrix[i - 2][j - 2] + 1;
                if (source[i - 2] != t_j) {trans++;}
                if (s_i != target[j - 2]) {trans++;}
                if (cell > trans) {cell = trans;}
            }

            matrix[i][j] = cell;
        }
    }

    return matrix[n][m];
}
