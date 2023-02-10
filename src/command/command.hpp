// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef COMMAND_CLASS_H_
#define COMMAND_CLASS_H_

#include "utils.hpp"

/**
 * @brief Class for executing a single command
 */
class Command
{
    private:

        /** @brief Pointer to the Device class object */
        Device * device;
        
    public:

        /**
         * @brief Construct a new Command object.
         * 
         * Will initialise a host (master) interface.
         * 
         * @param _dev          Pointer to the Device class object
         */
        Command(Device * _dev);

        /**
         * @brief Takes argv and executes a single command from it
         * 
         * @param cmd           Pointer to the command instance to be executed
         * @param argv          Pointer to command line arguments
         * @param argc          Number of arguments in command line
         * @param arg_indx      Index of argv to look at
         */
        control_ret_t do_command(const cmd_t * cmd, char ** argv, int argc, int arg_indx);

        /**
         * @brief Executes a single get comamnd
         * 
         * @param cmd           Pointer to the command instance to be executed
         * @param values        Pointer to store values read from the device
         */
        control_ret_t command_get(const cmd_t * cmd, cmd_param_t * values);

        /**
         * @brief Executes a single set command
         * 
         * @param cmd           Pointer to the command instance to be executed
         * @param values        Pointer to store values to write to the device
         */
        control_ret_t command_set(const cmd_t * cmd, const cmd_param_t * values);

        /**
         * @brief Low level get command function.
         *
         * This function sends a read command to the device and returns the error
         * returned from the device.
         * 
         * @param data          Byte array containing the read command
         * @note                Only for internal testing.
         */
        control_ret_t command_get_low_level(uint8_t *data);

        /**
         * @brief Low level set command function
         *
         * This function sends a write command and payload to the device and
         * returns the error code returned from the device.
         *
         * @param cmd           Byte array containing the write command and payload
         * @note                Only for internal testing
         */
        control_ret_t command_set_low_level(uint8_t *data);
};

#endif
