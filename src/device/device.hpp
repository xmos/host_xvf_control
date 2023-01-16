// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef DEVICE_CLASS_H_
#define DEVICE_CLASS_H_

extern "C"
#include "device_control_shared.h"
#include <memory>
#include <iostream>

#if defined(_WIN32)
#include <windows.h>
/** @brief Typedef for storing dynamically opened library  */
typedef HMODULE dl_handle_t;
#elif (defined(__APPLE__) || defined(__linux__))
/** @brief Typedef for storing dynamically opened library  */
typedef void * dl_handle_t;
#endif

/**
 * @brief Class for interfacing device_contol
 */
class Device
{
    private:

        /** @brief Information to intialise the device */
        int * device_info;

        /**
         * @brief State of the device.
         * 
         * Used to prevent multiple initialisations / cleanups.
         */
        bool device_initialised = false;

        /**
         * @brief Get the device_info pointer from the command_map shared object
         * 
         * @param handle    Pointer to the command_map shared object
         * @param symbol    Fuction name to get the device_info from
         */
        void get_device_info(dl_handle_t handle, const std::string symbol);

    public:

        /**
         * @brief Construct a new Device object
         * 
         * @param handle    Pointer to the command_map shared object
         */
        Device(dl_handle_t handle);

        /** @brief Initialise a host (master) interface */
        virtual control_ret_t device_init();

        /**
         * @brief Request to read from a controllable resource inside the device
         * 
         * @param res_id        Resource ID
         * @param cmd_id        Command ID
         * @param payload       Array of bytes which constitutes the data payload
         * @param payload_len   Size of the payload in bytes
         */
        virtual control_ret_t device_get(control_resid_t res_id, control_cmd_t cmd_id, uint8_t payload[], size_t payload_len);

        /**
         * @brief Request to write to a controllable resource inside the device
         * 
         * @param res_id        Resource ID
         * @param cmd_id        Command ID
         * @param payload       Array of bytes which constitutes the data payload
         * @param payload_len   Size of the payload in bytes
         */
        virtual control_ret_t device_set(control_resid_t res_id, control_cmd_t cmd_id, const uint8_t payload[], size_t payload_len);

        /**
         * @brief Destroy the Device object.
         * 
         * This will shut down the host interface connection.
         */
        virtual ~Device();
};

//extern "C"
/**
 * @brief Return unique pointer to the Device class object
 * 
 * @param handle    Pointer to the command_map shared object
 */
//std::unique_ptr<Device> make_Dev(void * handle);

extern "C"
#if defined(_WIN32)
__declspec(dllexport)
#endif
Device * make_Dev(dl_handle_t handle);

#endif
