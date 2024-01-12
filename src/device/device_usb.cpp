// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "device.hpp"
#include "device_control_host.h"

using namespace std;

Device::Device(int * info)
{
    device_info = info;
}

control_ret_t Device::device_init()
{
    control_ret_t ret = CONTROL_ERROR;
    if(!device_initialised)
    {
        for(int pid_idx = 0; pid_idx < 2; ++pid_idx)
        {
            ret = control_init_usb(static_cast<int>(device_info->vendor_id), static_cast<int>(device_info->product_id[pid_idx]), static_cast<int>(device_info->ctrl_if_num));
            if(ret == CONTROL_SUCCESS)
            {
                device_initialised = true;
                break;
            }
        }
    }
    else
    {
        ret = CONTROL_SUCCESS;
    }
    return ret;
}

control_ret_t Device::device_get(control_resid_t res_id, control_cmd_t cmd_id, uint8_t payload[], size_t payload_len)
{
    control_ret_t ret = control_read_command(res_id, cmd_id, payload, payload_len);
    return ret;
}

control_ret_t Device::device_set(control_resid_t res_id, control_cmd_t cmd_id, const uint8_t payload[], size_t payload_len)
{
    control_ret_t ret = control_write_command(res_id, cmd_id, payload, payload_len);
    return ret;
}

Device::~Device()
{
    if(device_initialised)
    {
        control_cleanup_usb();
        device_initialised = false;
    }
}

extern "C"
Device * make_Dev(int * info)
{
    static Device dev_obj(info);
    return &dev_obj;
}
