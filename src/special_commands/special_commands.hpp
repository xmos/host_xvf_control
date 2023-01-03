// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef SPECIAL_COMMANDS_H_
#define SPECIAL_COMMANDS_H_

#include "command.hpp"

/** @brief Load libcommand_map shared object and get the cmd_t array from it */
void * load_command_map_dll();

/**
 * @brief Lookup the string in the command list.
 * 
 * Function is case insensitive. If the string is not found, will
 * suggest a possible match and exit.
 * 
 * @param str   String sequence to lookup
 */
cmd_t * command_lookup(const std::string str);

/**
 * @brief Lookup the string in the option list.
 * 
 * Function is case insensitive. If the string is not found, will
 * suggest a possible match and exit.
 * 
 * @param str   String sequence to lookup
 */
opt_t * option_lookup(const std::string str);

/** @brief Print application help menu in stdout */
control_ret_t print_help_menu();

/** 
 * @brief Print command list loaded from libcommand_map in stdout 
 * 
 * @note Commands starting with SPECIAL_CMD_ and TEST_ will not be printed
 */
control_ret_t print_command_list();

/** 
 * @brief Do a read of all possible read commands and dump it into stdout
 * 
 * @param command   Pointer to the Command class object
 * @note Commands starting with SPECIAL_CMD_ and TEST_ will not be executed
 */
control_ret_t dump_params(Command * command);

/**
 * @brief Execute commands from .txt file.
 * 
 * Will execute one command per line.
 * 
 * @param command   Pointer to the Command class object
 * @param filename  Filename to read from
 * @note If flename is not specified will look for 'commands.txt'
 * @note Don't use --use inside .txt file
 */
control_ret_t execute_cmd_list(Command * command, const char * filename = "commands.txt");

/**
 * @brief Set or get AEC filter
 * 
 * @param command           Pointer to the Command class object
 * @param flag_buffer_get   Boolean to specify read/write operation
 * @param filename          Filename to read from/write to
 * @note Default filename is aec_filter.bin
 * @note This function will use separate files for each mic and farend channels
 * @note So each filename will be aec_filter.bin.fx.mx
 */
control_ret_t special_cmd_aec_filter(Command * command, bool flag_buffer_get, const char * filename = "aec_filter.bin\0");

/**
 * @brief Set or get Non-Linear model
 * 
 * @param command           Pointer to the Command class object
 * @param flag_buffer_get   Boolean to specify read/write operation
 * @param filename          Filename to read from/write to
 * @note Default filename is nlm_buffer.bin
 * @note This function will get number of rows and columns from the device
 * @note and expect filename to be nml_buffer.bin.rx.cx
 */
control_ret_t special_cmd_nlmodel_buffer(Command * command, bool flag_buffer_get, const char * filename = "nlm_buffer.bin\0");

/**
 * @brief Function to test control interface.
 * 
 * Will send and get 50 frames of 60 bytes. Input buffer should be named 'test_input_buf.bin'
 * If 'test_input_buf.bin' and output binary are bitexact, control interface is fine.
 * 
 * @param command       Pointer to the Command class object
 * @param filename      Filename to use for an output buffer
 */
control_ret_t test_control_interface(Command * command, const char * filename = "test_buffer.bin\0");

#endif
