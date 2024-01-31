// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "dfu_commands.hpp"
#include <iomanip>          // setprecision

/**
 * @brief Converts DFU state value into a string
 * @param state         Value of the DFU state
 *
 * @return              String representation of the DFU state
 */
const std::string dfu_state_to_string( int state );

/**
 * @brief Converts DFU status value into a string
 * @param state         Value of the DFU status
 *
 * @return              String representation of the DFU status
 */
const std::string dfu_status_to_string(int status);

/**
 * @brief Executes a GETSTATUS request
 *
 * @param device        Pointer to the Device class object
 * @param command_list  Pointer to the CommandList class object
 * @param status        Status read from the device
 * @param state         State read from the device
 * @param is_verbose    Flag to indicate if verbose mode is enabled
 *
 * @return              device control status
 */
control_ret_t get_status(Device * device, CommandList* command_list, uint8_t &status, uint8_t &state, uint8_t is_verbose);

/**
 * @brief Executes a CLRSTATUS request
 * @param device        Pointer to the Device class object
 * @param command_list  Pointer to the CommandList class object
 * @param is_verbose    Flag to indicate if verbose mode is enabled

 * @return              device control status
 */
control_ret_t clear_status(Device * device, CommandList* command_list, uint8_t is_verbose);

/**
 * @brief Checks if device is in dfuIDLE state
 * @param device        Pointer to the Device class object
 * @param command_list  Pointer to the CommandList class object
 * @param is_verbose    Flag to indicate if verbose mode is enabled
 *
 * @return              1 if it the state is dfuIDLE, 0 otherwise
 */
uint32_t state_is_idle(Device * device, CommandList* command_list, uint8_t is_verbose);

/**
 * @brief Executes a SETALTERNATE request
 *
 * @param device        Pointer to the Device class object
 * @param command_list  Pointer to the CommandList class object
 * @param alternate     Alt setting to use
 * @param is_verbose    Flag to indicate if verbose mode is enabled
 *
 * @return              device control status
 */
control_ret_t set_alternate(Device * device, CommandList* command_list, uint8_t alternate, uint8_t is_verbose);

/**
 * @brief Executes a download operation
 *
 * @param device        Pointer to the Device class object
 * @param command_list  Pointer to the CommandList class object
 * @param image_path    Path to the image to download to the device
 * @param is_verbose    Flag to indicate if verbose mode is enabled
 *
 * @return              device control status
 */
control_ret_t download_operation(Device * device, CommandList* command_list, const std::string image_path, uint8_t is_verbose);

/**
 * @brief Executes a download operation
 *
 * @param device        Pointer to the Device class object
 * @param command_list  Pointer to the CommandList class object
 * @param image_path    Path to the image to upload from the device
 * @param is_verbose    Flag to indicate if verbose mode is enabled
 *
 * @return              device control status
 */
control_ret_t upload_operation(Device * device, CommandList* command_list, const std::string image_path, uint8_t is_verbose);

/**
 * @brief Executes a reboot operation
 *
 * @param device        Pointer to the Device class object
 * @param command_list  Pointer to the CommandList class object
 * @param is_verbose    Flag to indicate if verbose mode is enabled
 *
 * @return              device control status
 */
control_ret_t reboot_operation(Device * device, CommandList* command_list, uint8_t is_verbose);

/**
 * @brief Sets a transport block number
 *
 * @param device        Pointer to the Device class object
 * @param command_list  Pointer to the CommandList class object
 * @param block_number  Transport block number to set
 * @param is_verbose    Flag to indicate if verbose mode is enabled
 *
 * @return              device control status
 */
control_ret_t set_transport_block(Device * device, CommandList* command_list, uint16_t block_number, uint8_t is_verbose);

/**
 * @brief Prints version of the device
 *
 * @param device        Pointer to the Device class object
 * @param command_list  Pointer to the CommandList class object
 * @param is_verbose    Flag to indicate if verbose mode is enabled
 *
 * @return              device control status
 */
control_ret_t get_version(Device * device, CommandList* command_list, uint8_t is_verbose);
