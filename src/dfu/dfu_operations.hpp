#include <chrono>
#include <thread>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>         // readlink
#include <sys/ioctl.h>      // ioctl
#include <iomanip>          // setprecision
#include "dfu_commands.hpp"

/** @brief Convert DFU state value into a string */
const std::string dfu_state_to_string( int state );

/** @brief Convert DFU status value into a string */
const std::string dfu_status_to_string(int status);

/**
 * @brief Executes a GETSTATUS request
 *
 * @param device        Pointer to the Device class object
 * @param status        Status read from the device
 * @param state         State read from the device
 *
 * @return              device control status
 */
control_ret_t getstatus(Device * device, CommandList* command_list, uint8_t &status, uint8_t &state, uint8_t is_verbose);

/**
 * @brief Executes a CLRSTATUS request
 *
 * @return              device control status
 */
control_ret_t clearStatus(Device * device, CommandList* command_list, uint8_t is_verbose);

/**
 * @brief Checks if device is in dfuIDLE state
 *
 * @param device        Pointer to the Device class object
 *
 * @return              1 if it the state is dfuIDLE, 0 otherwise
 */
uint32_t status_is_idle(Device * device, CommandList* command_list, uint8_t is_verbose);

/**
 * @brief Executes a SETALTERNATE request
 *
 * @param device        Pointer to the Device class object
 * @param alternate     Alt setting to use
 *
 * @return              device control status
 */
control_ret_t setalternate(Device * device, CommandList* command_list, uint8_t alternate, uint8_t is_verbose);

/**
 * @brief Executes a download operation
 *
 * @param device        Pointer to the Device class object
 * @param image_path    Path to the image to download to the device
 *
 * @return              device control status
 */
control_ret_t download_operation(Device * device, CommandList* command_list, const std::string image_path, uint8_t is_verbose);

/**
 * @brief Executes a reboot operation
 *
 * @param device        Pointer to the Device class object
 *
 * @return              device control status
 */
control_ret_t reboot_operation(Device * device, CommandList* command_list, uint8_t is_verbose);

/**
 * @brief Executes a download operation
 *
 * @param device        Pointer to the Device class object
 * @param image_path    Path to the image to upload from the device
 *
 * @return              device control status
 */
control_ret_t upload_operation(Device * device, CommandList* command_list, const std::string image_path, uint8_t is_verbose);

/*
 * @brief Set a transport block number
 *
 * @param device        Pointer to the Device class object
 * @param block_number  Transport block number to set
 *
 * @return              device control status
 */
control_ret_t set_transport_block(Device * device, CommandList* command_list, uint16_t block_number, uint8_t is_verbose);

/**
 * @brief Print version of the device
 *
 * @param device        Pointer to the Device class object
 *
 * @return              device control status
 */
control_ret_t getversion(Device * device, CommandList* command_list, uint8_t is_verbose);

/**
 * @brief Set a transport block number
 *
 * @param device        Pointer to the Device class object
 * @param block_number  Transport block number to set
 *
 * @return              device control status
 */
control_ret_t set_transport_block(Device * device, CommandList* command_list, uint16_t block_number, uint8_t is_verbose);
