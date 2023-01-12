// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "device.hpp"
#include <cstring>
#include <dlfcn.h>
#include <fstream>

using namespace std;


const size_t num_vals = 20;
uint8_t buffer[num_vals * sizeof(float)] = {0};
const string buf_filename = "test_buf.bin";
const size_t buff_size = end(buffer) - begin(buffer);

Device::Device(void * handle)
{
    // declaring int * function pointer type
    using info_t = int * (*)();
    info_t info = reinterpret_cast<info_t>(dlsym(handle, "get_info"));
    if(info == NULL)
    {
        cerr << "Error while loading get_info from command_map" << endl;
        exit(CONTROL_ERROR);
    }
    device_info = info();
}

control_ret_t Device::device_init()
{
    control_ret_t ret = CONTROL_SUCCESS;
    if(!device_initialised)
    {
        char check[4];
        memcpy(check, device_info, sizeof(int32_t));
        string check_str = check;
        if(check_str != "test")
        {
            ret = CONTROL_REGISTRATION_FAILED;
        }
        device_initialised = true;
    }
    return ret;
}

control_ret_t Device::device_get(control_resid_t res_id, control_cmd_t cmd_id, uint8_t payload[], size_t payload_len)
{
    if(payload_len > (buff_size + 1)) // Max buffer len + 1 for the status
    {
        return CONTROL_DATA_LENGTH_ERROR;
    }
    payload[0] = 0;

    ifstream rf(buf_filename, ios::out | ios::binary);
    if(!rf)
    {
        cerr << "Could not open a file " << buf_filename << endl;
        return CONTROL_ERROR;
    }

    rf.seekg (0, rf.end);
    int32_t size = rf.tellg();
    rf.seekg (0, rf.beg);

    if(size != buff_size)
    {
        cerr << "Test buffer lengths don't match" << endl;
        return CONTROL_DATA_LENGTH_ERROR;
    }

    for(int i = 0; i < buff_size; i++)
    {
        rf.read(reinterpret_cast<char *>(&buffer[i]), sizeof(uint8_t));
    }

    rf.peek(); // doing peek here to look for a character beyond file size so it will set eof
    rf.close();

    if(!rf.eof() || rf.bad())
    {
        cerr << "Error occured while reading " << buf_filename << endl;
        exit(CONTROL_ERROR);
    }

    switch(cmd_id & 0x7F)
    {
    case 0:
        memcpy(&payload[1], buffer, num_vals * sizeof(float));
    case 1:
        memcpy(&payload[1], buffer, num_vals * sizeof(uint8_t));
    }
    return CONTROL_SUCCESS;
}

control_ret_t Device::device_set(control_resid_t res_id, control_cmd_t cmd_id, const uint8_t payload[], size_t payload_len)
{
    if(payload_len > buff_size) // Max buffer len
    {
        return CONTROL_DATA_LENGTH_ERROR;
    }

    switch(cmd_id)
    {
    case 0:
        memcpy(buffer, payload, num_vals * sizeof(float));
    case 1:
        memcpy(buffer, payload, num_vals * sizeof(uint8_t));
    }

    ofstream wf(buf_filename, ios::out | ios::binary);
    if(!wf)
    {
        cerr << "Could not open a file " << buf_filename << endl;
        return CONTROL_ERROR;
    }

    for(int i = 0; i < buff_size; i++)
    {
        wf.write(reinterpret_cast<char *>(&buffer[i]), sizeof(uint8_t));
    }

    wf.close();
    if(wf.bad())
    {
        cerr << "Error occured when writing to " << buf_filename << endl;
        return CONTROL_ERROR;
    }

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
