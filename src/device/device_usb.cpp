#include "device.hpp"
#include "device_control_host.h"

using namespace std;

extern "C"{
control_ret_t control_init_usb(int vendor_id, int product_id, int interface_num);
control_ret_t control_cleanup_usb(void);
control_ret_t control_write_command(control_resid_t resid, control_cmd_t cmd,
                      const uint8_t payload[], size_t payload_len);
control_ret_t control_read_command(control_resid_t resid, control_cmd_t cmd,
                     uint8_t payload[], size_t payload_len);
}

Device::Device(dl_handle_t handle)
{
    get_device_info(handle , "get_info_usb");
}

control_ret_t Device::device_init()
{
    control_ret_t ret = CONTROL_SUCCESS;
    if(!device_initialised)
    {
        ret = control_init_usb(static_cast<int>(device_info[0]), static_cast<int>(device_info[1]), static_cast<int>(device_info[2]));
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
        control_cleanup_usb();
        device_initialised = false;
    }
}

extern "C"
Device * make_Dev(dl_handle_t handle)
{
    static Device dev_obj(handle);
    return &dev_obj;
}