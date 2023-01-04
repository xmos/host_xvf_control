// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "utils.hpp"

#if defined(__unix__)
#include <dlfcn.h>
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

using namespace std;

string get_dynamic_lib_path(string lib_name)
{
    lib_name = "lib" + lib_name;
#ifdef __unix__
    char * dir_path;
    char path[PATH_MAX];
    lib_name = '/' + lib_name;
#if defined(__linux__)
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count == -1)
    {
        cerr << "Could not read /proc/self/exe into " << PATH_MAX << " string" << endl;
        exit(CONTROL_ERROR);
    }
    lib_name += ".so";
#elif defined(__APPLE__)
    uint32_t size = PATH_MAX;
    if (_NSGetExecutablePath(path, &size) != 0)
    {
        cerr << "Could not get binary path into " << PATH_MAX << " string" << endl;
        exit(CONTROL_ERROR);
    }
    lib_name += ".dylib"
#endif  // linux vs mac
#elif defined(_WIN32)
    lib_name = '\\' + lib_name;
    char path[MAX_PATH];
    if(0 == GetModuleFileNameA(GetModuleHandle(NULL), path, MAX_PATH))
    {
        cerr << "Could not get binary path into " << MAX_PATH << " string" << endl;
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

void * get_dynamic_lib(string lib_name)
{
    string dyn_lib_path = get_dynamic_lib_path(lib_name);
#ifdef __unix__
    static_cast<void>(dlerror()); // clear errors
    void * handle = dlopen(dyn_lib_path.c_str(), RTLD_NOW);
#elif defined(_WIN32)
#error "Windows is currently not supported"
#else
#error "Unknown Operating System"
#endif
    if(handle == NULL)
    {
#ifdef __unix__
        cerr << dlerror() << endl;
#elif defined(_WIN32)
#error "Windows is currently not supported"
#else
#error "Unknown Operating System"
#endif
        exit(CONTROL_ERROR);
    }
    return handle;
}

template<typename T>
T get_function(void * handle, string symbol)
{
#ifdef __unix__
    static_cast<void>(dlerror()); // clear errors
    T func = reinterpret_cast<T>(dlsym(handle, symbol.c_str()));
#elif defined(_WIN32)
#error "Windows is currently not supported"
#else
#error "Unsupported operating system"
#endif
    if(func == NULL)
    {
#ifdef __unix__
    cerr << dlerror() << endl;
#elif defined(_WIN32)
#error "Windows is currently not supported"
#else
#error "Unsupported operating system"
#endif
        exit(CONTROL_ERROR);
    }
    return func;
}

cmd_map_fp get_cmd_map_fp(void * handle)
{
    return get_function<cmd_map_fp>(handle, "get_command_map");
}

num_cmd_fp get_num_cmd_fp(void * handle)
{
    return get_function<num_cmd_fp>(handle, "get_num_commands");
}

device_fp get_device_fp(void * handle)
{
    return get_function<device_fp>(handle, "make_Dev");
}