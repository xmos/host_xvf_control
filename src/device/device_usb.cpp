// Copyright 2022-2024 XMOS LIMITED.
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
        // The USB device information list has a peculiar structure.
        // It consists of multiple sets.
        // Each set has three members, a VID, a PID, and the number of the USB control interface.
        // These three members appear in the order given above.
        // To allow the count of sets to change in future, the zero-th element in the list holds a count of how many sets follow.
        int info_set_count = static_cast<int>(device_info[0]);
        for(int set_idx = 0; set_idx < info_set_count; ++set_idx)
        {
            int offset = set_idx * 3; // Three members per set
            ret = control_init_usb(static_cast<int>(device_info[offset+1]), static_cast<int>(device_info[offset+2]), static_cast<int>(device_info[offset+3]));
            if(ret == CONTROL_SUCCESS)
            {
                device_initialised = true;
                cout << "Device (USB)::device_init() -- Found device VID: " << device_info[offset+1] << " PID: " << device_info[offset+2] << " interface: " << device_info[offset+3] << endl;
                break;
            }
            cerr << "Device (USB)::device_init() -- No device found" << endl;
        }
    }
    else
    {
        cerr << "Device (USB)::device_init() -- Device already initialised" << endl;
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
