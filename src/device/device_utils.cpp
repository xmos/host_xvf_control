// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "device.hpp"

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

void report_error()
{
#if (defined(__linux__) || defined(__APPLE__))
    cerr << dlerror() << endl;
#elif defined(_WIN32)
    cerr << GetLastError() << endl;
#else
#error "Unknown Operating System"
#endif // unix vs windows
}

void Device::get_device_info(dl_handle_t handle, const string symbol)
{
    using device_info_fptr = int * (*)();
#if (defined(__linux__) || defined(__APPLE__))
    static_cast<void>(dlerror()); // clear errors
    device_info_fptr func = reinterpret_cast<device_info_fptr>(dlsym(handle, symbol.c_str()));
#elif defined(_WIN32)
    device_info_fptr func = reinterpret_cast<device_info_fptr>(GetProcAddress(handle, symbol.c_str()));
#else
#error "Unsupported operating system"
#endif // unix vs windows
    if(func == NULL)
    {
        report_error();
        exit(CONTROL_ERROR);
    }
    device_info = func();
}
