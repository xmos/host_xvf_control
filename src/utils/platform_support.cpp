// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "utils.hpp"

#if defined(__linux__)
#include <dlfcn.h>          // dlopen/dlsym/dlerror/dlclose
#include <unistd.h>         // readlink
#include <linux/limits.h>   // PATH_MAX

#elif defined(__APPLE__)
#include <dlfcn.h>          // dlopen/dlsym/dlerror/dlclose
#include <unistd.h>         // readlink
#include <mach-o/dyld.h>    // _NSGetExecutablePath

#elif defined(_WIN32)
#include <Windows.h>        // GetModuleFileNameA
#include <errhandlingapi.h> // GetLastError

#else
#error "Unknown Operating System"
#endif

using namespace std;

string get_dynamic_lib_path(const string lib_name)
{

#if defined(__linux__)
    string full_lib_name = "lib" + lib_name;
    char * dir_path;
    char path[PATH_MAX];
    full_lib_name = '/' + full_lib_name;
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count == -1)
    {
        cerr << "Could not read /proc/self/exe into " << PATH_MAX << " string" << endl;
        exit(CONTROL_ERROR);
    }
    full_lib_name += ".so";
#elif defined(__APPLE__)
    string full_lib_name = "lib" + lib_name;
    char * dir_path;
    char path[PATH_MAX];
    full_lib_name = '/' + full_lib_name;
    uint32_t size = PATH_MAX;
    if (_NSGetExecutablePath(path, &size) != 0)
    {
        cerr << "Could not get binary path into " << PATH_MAX << " string" << endl;
        exit(CONTROL_ERROR);
    }
    full_lib_name += ".dylib";
#elif defined(_WIN32)
    string full_lib_name = lib_name;
    full_lib_name = '\\' + full_lib_name;
    char path[MAX_PATH];
    if(0 == GetModuleFileNameA(GetModuleHandle(NULL), path, MAX_PATH))
    {
        cerr << "Could not get binary path into " << MAX_PATH << " string" << endl;
        exit(CONTROL_ERROR);
    }
    full_lib_name += ".dll";
#endif // linux vs apple vs windows
    string full_path = path;
    size_t found = full_path.find_last_of("/\\"); // works for both unix and windows
    string dir_path_str = full_path.substr(0, found);
    string lib_path_str = dir_path_str + full_lib_name;

    return lib_path_str;
}

dl_handle_t get_dynamic_lib(const string lib_name)
{
    string dyn_lib_path = get_dynamic_lib_path(lib_name);
#if (defined(__linux__) || defined(__APPLE__))
    static_cast<void>(dlerror()); // clear errors
    dl_handle_t handle = dlopen(dyn_lib_path.c_str(), RTLD_NOW);
#elif defined(_WIN32)
    dl_handle_t handle = LoadLibrary(dyn_lib_path.c_str());
#else
#error "Unknown Operating System"
#endif // unix vs windows
    if(handle == NULL)
    {
#if (defined(__linux__) || defined(__APPLE__))
        cerr << dlerror() << endl;
#elif defined(_WIN32)
        cerr << "Could not load " << lib_name << ", got " << GetLastError() << " error code" << endl;
#else
#error "Unknown Operating System"
#endif // unix vs windows        
        exit(CONTROL_ERROR);
    }
    return handle;
}

template<typename T>
T get_function(dl_handle_t handle, const string symbol)
{
#if (defined(__linux__) || defined(__APPLE__))
    static_cast<void>(dlerror()); // clear errors
    T func = reinterpret_cast<T>(dlsym(handle, symbol.c_str()));
#elif defined(_WIN32)
    T func = reinterpret_cast<T>(GetProcAddress(handle, symbol.c_str()));
#else
#error "Unsupported operating system"
#endif // unix vs windows
    if(func == NULL)
    {
#if (defined(__linux__) || defined(__APPLE__))
        cerr << dlerror() << endl;
#elif defined(_WIN32)
        cerr << "Could not load " << symbol << " function, got " << GetLastError() << "error code" << endl;
#else
#error "Unknown Operating System"
#endif // unix vs windows
        exit(CONTROL_ERROR);
    }
    return func;
}

cmd_map_fptr get_cmd_map_fptr(dl_handle_t handle)
{
    return get_function<cmd_map_fptr>(handle, "get_command_map");
}

num_cmd_fptr get_num_cmd_fptr(dl_handle_t handle)
{
    return get_function<num_cmd_fptr>(handle, "get_num_commands");
}

device_fptr get_device_fptr(dl_handle_t handle)
{
    return get_function<device_fptr>(handle, "make_Dev");
}
