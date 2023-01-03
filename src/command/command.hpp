// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef COMMAND_CLASS_H_
#define COMMAND_CLASS_H_

#include "device.hpp"
#include "utils.hpp"

/**
 * @brief Class for sending single commands to the device
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
         * @param args_left     Number of arguments left in command line
         * @param arg_indx      Index of argv to look at
         */
        control_ret_t do_command(cmd_t * cmd, char ** argv, int args_left, int arg_indx);

        /**
         * @brief Executes single get comamnd
         * 
         * @param cmd           Pointer to the command instance to be executed
         * @param values        Pointer to the array of union values to fill from the device
         */
        control_ret_t command_get(cmd_t * cmd, cmd_param_t * values);

        /**
         * @brief Executes single set command
         * 
         * @param cmd           Pointer to the command instance to be executed
         * @param values        Pointer to the array of union values to write to the device
         */
        control_ret_t command_set(cmd_t * cmd, const cmd_param_t * values);
};

#endif
