// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "factory.hpp"

using namespace std;

factory::factory(const char * filename) : handle(dlopen(filename, RTLD_NOW | RTLD_LOCAL))
{
    if (handle == NULL) throw factory_error(string(dlerror()));
    make_dev = load<device_t>("make_Dev");
}
