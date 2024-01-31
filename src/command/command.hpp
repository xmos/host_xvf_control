// Copyright 2022-2024 XMOS LIMITED.
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

        /** @brief Command information */
        cmd_t cmd;

        /** @brief Bypass range check state */
        bool bypass_range_check;

        /** @brief Pointer to the super_print_arg() function from the command_map shared object */
        print_args_fptr print_args;

        /** @brief Pointer to the check_range() function from the command_map shared object */
        check_range_fptr check_range;

    public:

        /**
         * @brief Construct a new Command object.
         *
         * Will initialise a host (master) interface.
         *
         * @param _dev          Pointer to the Device class object
         * @param _bypass_range Bypass range check state
         * @param _handle       Command map dl handle
         */
        Command(Device * _dev, bool _bypass_range, dl_handle_t _handle);

        /**
         * @brief Initialise command information
         *
         * @param cmd_name      The command name to be executed
         * @note This has to be used if using command_get() or command_set()
         */
        void init_cmd_info(const std::string cmd_name);

        /**
         * @brief Takes argv and executes a single command from it
         *
         * @param cmd_name      The command name to be executed
         * @param argv          Pointer to command line arguments
         * @param argc          Number of arguments in command line
         * @param arg_indx      Index of argv to look at
         */
        control_ret_t do_command(const std::string cmd_name, char ** argv, int argc, int arg_indx);

        /**
         * @brief Executes a single get command
         *
         * @param values        Pointer to store values read from the device
         */
        control_ret_t command_get(cmd_param_t * values);

        /**
         * @brief Executes a single set command
         *
         * @param values        Pointer to store values to write to the device
         */
        control_ret_t command_set(const cmd_param_t * values);

        /**
         * @brief Low level get command function.
         *
         * This function sends a read command to the device and returns the error
         * returned from the device.
         *
         * @param data          Byte array containing the read command
         * @param payload_len   Length of the byte stream to write to the device
         * @note                Only for internal testing.
         */
        control_ret_t command_get_low_level(uint8_t *data, size_t payload_len);

        /**
         * @brief Low level set command function
         *
         * This function sends a write command and payload to the device and
         * returns the error code returned from the device.
         *
         * @param cmd           Byte array containing the write command and payload
         * @param payload_len   Length of the byte stream to write to the device
         * @note                Only for internal testing
         */
        control_ret_t command_set_low_level(uint8_t *data, size_t payload_len);

        /**
         * @brief Convert single value from bytes to cmd_param_t
         *
         * @param data          Byte array containing the data to read
         * @param index         Position of the first byte to use
         */
        cmd_param_t command_bytes_to_value(const uint8_t * data, unsigned index);

        /**
         * @brief Convert single value from cmd_param_t to bytes
         *
         * @param data          Byte array containing the data to write
         * @param index         Position of the first byte to use
         * @param value         Value to convert
         */
        void command_bytes_from_value(uint8_t * data, unsigned index, const cmd_param_t value);

        /**
         * @brief Get number of bytes for the particular param type
         */
        size_t get_num_bytes_from_type();

        /**
         * @brief Convert command line argument from string to cmd_param_t
         *
         * @param str           String storing the command line argument to convert
         */
        cmd_param_t cmd_arg_str_to_val(const char * str);
};

/**
 * @brief Get string with type name for the particular param type
 *
 * @param type          Command type
 */
std::string command_param_type_name(cmd_param_type_t type);

#endif
