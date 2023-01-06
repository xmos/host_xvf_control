// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "device.hpp"
#include <cstring>
#include <dlfcn.h>

using namespace std;

float buffer[60] = {0.0};

Device::Device(void * handle)
{
    // declaring int * function pointer type
    using info_t = int * (*)();
    info_t info = reinterpret_cast<info_t>(dlsym(handle, "get_info_i2c"));
    if(info == NULL)
    {
        cerr << "Error while loading get_info_i2c() from libcommand_map" << endl;
        exit(CONTROL_ERROR);
    }
    device_info = info();
}

control_ret_t Device::device_init()
{
    control_ret_t ret = CONTROL_SUCCESS;
    if(!device_initialised)
    {
        device_initialised = true;
    }
    return ret;
}

control_ret_t Device::device_get(control_resid_t res_id, control_cmd_t cmd_id, uint8_t payload[], size_t payload_len)
{
    if(payload_len != 60)
    {
        return CONTROL_DATA_LENGTH_ERROR;
    }
    memcpy(payload, buffer, 60 * sizeof(float));
    return CONTROL_SUCCESS;
}

control_ret_t Device::device_set(control_resid_t res_id, control_cmd_t cmd_id, const uint8_t payload[], size_t payload_len)
{
    if(payload_len != 60)
    {
        return CONTROL_DATA_LENGTH_ERROR;
    }
    memcpy(buffer, payload, 60 * sizeof(float));
    return CONTROL_SUCCESS;
}

Device::~Device()
{
    if(device_initialised)
    {
        device_initialised = false;
    }
}

extern "C"
unique_ptr<Device> make_Dev(void * handle)
{
    return make_unique<Device>(handle);
}
