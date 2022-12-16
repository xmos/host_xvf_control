// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "factory.hpp"
#include <iostream>

using namespace std;

factory::factory(const char * filename) : handle(dlopen(filename, RTLD_NOW | RTLD_LOCAL))
{
    if(handle == NULL)
    {
        cout << "Error while opening " << filename << endl;
        exit(CONTROL_ERROR);
    }
    make_dev = load<device_t>("make_Dev");
    if(make_dev == NULL)
    {
        cout << "Error while loading make_Dev() from the device driver" << endl;
        exit(CONTROL_ERROR);
    }
}
