// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef DEVICE_CLASS_H_
#define DEVICE_CLASS_H_

extern "C"{
#include "device_control_shared.h"
}
#include <memory>

class Device
{
    private:
    bool device_initialised = false;

    public:
    virtual control_ret_t device_init();
    virtual control_ret_t device_get(control_resid_t res_id, control_cmd_t cmd_id, uint8_t payload[], size_t payload_len);
    virtual control_ret_t device_set(control_resid_t res_id, control_cmd_t cmd_id, const uint8_t payload[], size_t payload_len);
    virtual ~Device();
};

extern "C" {
Device * make_Dev();
}

#endif
