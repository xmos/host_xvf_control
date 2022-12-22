// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include <iostream>
#include "device.hpp"
#include "device_control_host.h"
#include "dlfcn.h"
#include "bcm2835.h"

using namespace std;

extern "C"{
control_ret_t control_init_spi_pi(spi_mode_t spi_mode, bcm2835SPIClockDivider clock_divider);
control_ret_t control_write_command(control_resid_t resid, control_cmd_t cmd,
                      const uint8_t payload[], size_t payload_len);
control_ret_t control_read_command(control_resid_t resid, control_cmd_t cmd,
                     uint8_t payload[], size_t payload_len);
control_ret_t control_cleanup_spi();
}

Device::Device(void * handle)
{
    // declaring int * function pointer type
    using info_t = int * (*)();
    info_t info = reinterpret_cast<info_t>(dlsym(handle, "get_info_spi"));
    if(info == NULL)
    {
        cerr << "Error while loading get_info_spi() from libcommand_map" << endl;
        exit(CONTROL_ERROR);
    }
    device_info = info();
}

control_ret_t Device::device_init()
{
    control_ret_t ret = CONTROL_SUCCESS;
    if(!device_initialised)
    {
        ret = control_init_spi_pi(static_cast<spi_mode_t>(device_info[0]), static_cast<bcm2835SPIClockDivider>(device_info[1]));
        device_initialised = true;
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
        control_cleanup_spi();
    }
}

extern "C"
unique_ptr<Device> make_Dev(void * handle)
{
    return make_unique<Device>(handle);
}
