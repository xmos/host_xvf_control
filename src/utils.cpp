// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "utils.hpp"
#include <cstring>
#include <cctype>
#include <vector>
#include <iomanip>

#if defined(__unix__)
#include <unistd.h>         // readlink
#if defined(__linux__)
#include <linux/limits.h>   // PATH_MAX
#elif defined(__APPLE__)
#include <mach-o/dyld.h>    // _NSGetExecutablePath
#else
#error "Unknown UNIX Operating System"
#endif // linux  vs mac
#elif defined(_WIN32)
#include <Windows.h>
#else
#error "Unknown Operating System"
#endif // unix vs windows

#define PI_VALUE  3.14159265358979323846f

using namespace std;

string get_dynamic_lib_path(string lib_name)
{
#ifdef __unix__
    char* dir_path;
    char path[PATH_MAX];
    lib_name = '/' + lib_name;
#if defined(__linux__)
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count == -1)
    {
        cout << "Could not read /proc/self/exe into " << PATH_MAX << " string." << endl;
        exit(CONTROL_ERROR);
    }
    lib_name += ".so";
#elif defined(__APPLE__)
    uint32_t size = PATH_MAX;
    if (_NSGetExecutablePath(path, &size) != 0)
    {
        cout << "Could not get binary path into " << PATH_MAX << " string." << endl;
        exit(CONTROL_ERROR);
    }
    lib_name += ".dylib"
#endif  // linux vs mac
#elif defined(_WIN32)
    lib_name = '\\' + lib_name;
    char path[MAX_PATH];
    if(0 == GetModuleFileNameA(GetModuleHandle(NULL), path, MAX_PATH))
    {
        cout << "Could not get binary path into " << MAX_PATH << " string." << endl;
        exit(CONTROL_ERROR);
    }
    lib_name += ".dll";
#endif
    string full_path = path;
    size_t found = full_path.find_last_of("/\\"); // works for both unix and windows
    string dir_path_str = full_path.substr(0, found);
    string lib_path_str = dir_path_str + lib_name;

    return lib_path_str;
}

string to_upper(string str)
{
    for(int i = 0; i < str.length(); i++)
    {
        str[i] = toupper(str[i]);
    }
    return str;
}

string to_lower(string str)
{
    for(int i = 0; i < str.length(); i++)
    {
        str[i] = tolower(str[i]);
    }
    return str;
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
        tstr = "WRITE ONLY";
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
        case TYPE_CHAR:
            cout << "TYPE_CHAR commands can only be READ_ONLY" << endl;
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
    }
}

unsigned get_num_bytes_from_type(cmd_param_type_t type)
{
    unsigned num_bytes;
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
        memcpy(&value.ui8, static_cast<uint8_t*>(data) + index * size_bytes, size_bytes);
        break;
    case 4:
        memcpy(&value.i32, static_cast<uint8_t*>(data) + index * size_bytes, size_bytes);
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
        memcpy(static_cast<uint8_t*>(data) + index * num_bytes, &value.ui8, num_bytes);
        break;
    case 4:
        memcpy(static_cast<uint8_t*>(data) + index * num_bytes, &value.i32, num_bytes);
        break;
    }
}


// This function implements algorithm to find Levenshtein Distance
// for approximate string matching. Taken from:
// https://www.talkativeman.com/levenshtein-distance-algorithm-string-comparison/
int levDistance(const string source, const string target)
{

  const int n = source.length();
  const int m = target.length();
  if (n == 0) {
    return m;
  }
  if (m == 0) {
    return n;
  }

  typedef vector<vector<int>> Tmatrix; 

  Tmatrix matrix(n+1);

  // Size the vectors in the 2.nd dimension. Unfortunately C++ doesn't
  // allow for allocation on declaration of 2.nd dimension of vec of vec

  for (int i = 0; i <= n; i++) {
    matrix[i].resize(m+1);
  }

  for (int i = 0; i <= n; i++) {
    matrix[i][0]=i;
  }

  for (int j = 0; j <= m; j++) {
    matrix[0][j]=j;
  }

  for (int i = 1; i <= n; i++) {

    const char s_i = source[i-1];

    for (int j = 1; j <= m; j++) {

      const char t_j = target[j-1];

      int cost;
      if (s_i == t_j) {
        cost = 0;
      }
      else {
        cost = 1;
      }

      const int above = matrix[i-1][j];
      const int left = matrix[i][j-1];
      const int diag = matrix[i-1][j-1];
      int cell = min( above + 1, min(left + 1, diag + cost));

      // Cover transposition, in addition to deletion,
      // insertion and substitution. This step is taken from:
      // Berghel, Hal ; Roach, David : "An Extension of Ukkonen's 
      // Enhanced Dynamic Programming ASM Algorithm"
      // (http://www.acm.org/~hlb/publications/asm/asm.html)

      if (i>2 && j>2) {
        int trans=matrix[i-2][j-2]+1;
        if (source[i-2]!=t_j) trans++;
        if (s_i!=target[j-2]) trans++;
        if (cell>trans) cell=trans;
      }

      matrix[i][j]=cell;
    }
  }

  return matrix[n][m];
}
