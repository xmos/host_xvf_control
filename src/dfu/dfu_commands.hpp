// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "utils.hpp"
#include <map>
#include <yaml-cpp/yaml.h>

/**
 * @brief DFU_GETSTATUS status values
 * @note These values are the same as section 6.1.2 of DFU Rev 1.1
 **/
#define DFU_STATUS_OK               0x00
#define DFU_STATUS_errTARGET        0x01
#define DFU_STATUS_errFILE          0x02
#define DFU_STATUS_errWRITE         0x03
#define DFU_STATUS_errERASE         0x04
#define DFU_STATUS_errCHECK_ERASED  0x05
#define DFU_STATUS_errPROG          0x06
#define DFU_STATUS_errVERIFY        0x07
#define DFU_STATUS_errADDRESS       0x08
#define DFU_STATUS_errNOTDONE       0x09
#define DFU_STATUS_errFIRMWARE      0x0a
#define DFU_STATUS_errVENDOR        0x0b
#define DFU_STATUS_errUSBR          0x0c
#define DFU_STATUS_errPOR           0x0d
#define DFU_STATUS_errUNKNOWN       0x0e
#define DFU_STATUS_errSTALLEDPKT    0x0f

/**
 * @brief DFU state values
 * @note These values are the same as section 6.1.2 of DFU Rev 1.1
 **/
#define DFU_STATE_appIDLE               0
#define DFU_STATE_appDETACH             1
#define DFU_STATE_dfuIDLE               2
#define DFU_STATE_dfuDNLOAD_SYNC        3
#define DFU_STATE_dfuDNBUSY             4
#define DFU_STATE_dfuDNLOAD_IDLE        5
#define DFU_STATE_dfuMANIFEST_SYNC      6
#define DFU_STATE_dfuMANIFEST           7
#define DFU_STATE_dfuMANIFEST_WAIT_RST  8
#define DFU_STATE_dfuUPLOAD_IDLE        9
#define DFU_STATE_dfuERROR              10

/** @brief Alt-setting values for flash partition */
#define DFU_ALT_FACTORY_ID 0
#define DFU_ALT_UPGRADE_ID 1

/** @brief Number of bytes used to indicate the length of the transfer block length */
#define DFU_TRANSFER_BLOCK_LENGTH_BYTES 2

/** @brief Invalid value for transport block number */
#define INVALID_TRANSPORT_BLOCK_NUM 0xFFFF

/** @brief List of strings for DFU statuses */
static const char *dfu_status_names[] = {
    /* DFU_STATUS_OK */
        "No error condition is present",
    /* DFU_STATUS_errTARGET */
        "File is not targeted for use by this device",
    /* DFU_STATUS_errFILE */
        "File is for this device but fails some vendor-specific test",
    /* DFU_STATUS_errWRITE */
        "Device is unable to write memory",
    /* DFU_STATUS_errERASE */
        "Memory erase function failed",
    /* DFU_STATUS_errCHECK_ERASED */
        "Memory erase check failed",
    /* DFU_STATUS_errPROG */
        "Program memory function failed",
    /* DFU_STATUS_errVERIFY */
        "Programmed memory failed verification",
    /* DFU_STATUS_errADDRESS */
        "Cannot program memory due to received address that is out of range",
    /* DFU_STATUS_errNOTDONE */
        "Received DFU_DNLOAD with wLength = 0, but device does not think that it has all data yet",
    /* DFU_STATUS_errFIRMWARE */
        "Device's firmware is corrupt. It cannot return to run-time (non-DFU) operations",
    /* DFU_STATUS_errVENDOR */
        "iString indicates a vendor specific error",
    /* DFU_STATUS_errUSBR */
        "Device detected unexpected USB reset signalling",
    /* DFU_STATUS_errPOR */
        "Device detected unexpected power on reset",
    /* DFU_STATUS_errUNKNOWN */
        "Something went wrong, but the device does not know what it was",
    /* DFU_STATUS_errSTALLEDPKT */
        "Device stalled an unexpected request"
};

/** @brief List of supported DFU commands */
static std::string commandNames[] =
{
    "DFU_DETACH",
    "DFU_DNLOAD",
    "DFU_UPLOAD",
    "DFU_GETSTATUS",
    "DFU_CLRSTATUS",
    "DFU_GETSTATE",
    "DFU_ABORT",
    "DFU_SETALTERNATE",
    "DFU_TRANSFERBLOCK",
    "DFU_GETVERSION",
    "DFU_REBOOT"
};

class CommandList
{
    /**
    * @brief Dictionaries storing command IDs of the DFU commands
-   * @note The values are read from the DFU yaml file
    **/
    std::map<std::string, int>CommandIDs;
    /**
    * @brief Dictionaries storing command lengths of the DFU commands
-   * @note The values are read from the DFU yaml file
    **/
    std::map<std::string, int>CommandLengths;
    /** @brief Resource ID of DFU controller servicer
    * @note The value is read from the DFU yaml file
    **/
    uint16_t dfu_controller_servicer_resid;

    public:

    /**
    * @brief Set function for command ID
    * @param cmd_name      Command name
    * @param id            Command ID
    **/
    void set_cmd_id(std::string cmd_name, int id) {CommandIDs[cmd_name] = id;};

    /**
    * @brief Set function for command length
    * @param cmd_name      Command name
    * @param length        Command length
    **/
    void set_cmd_length(std::string cmd_name, int length) {CommandLengths[cmd_name] = length;};

    /**
    * @brief Set function for DFU controller servicer resource ID
    * @param id            Resource ID
    **/
    void set_dfu_controller_servicer_resid(uint16_t res_id) {dfu_controller_servicer_resid = res_id;};

    /**
    * @brief Get function for command ID
    * @param cmd_name      Command name
    *
    * @return              Command ID
    **/
    int get_cmd_id(std::string cmd_name) {return CommandIDs[cmd_name];};

    /**
    * @brief Get function for command length
    * @param cmd_name      Command name
    *
    * @return              Command length
    **/
    int get_cmd_length(std::string cmd_name) {return CommandLengths[cmd_name];};
    /**
    * @brief Get function for DFU controller servicer resource ID
    *
    * @return               Resource ID
    **/
    uint16_t get_dfu_controller_servicer_resid() {return dfu_controller_servicer_resid;};

    /**
    * @brief Executes a single get command
     *
    * @param device        Pointer to the Device class object
    * @param cmd_name      Command name
    * @param values        Buffer storing the read values
    *
    * @return              device control status
    */
    control_ret_t command_get(Device * device, std::string cmd_name, uint8_t * values);

    /**
    * @brief Executes a single set command
     *
    * @param device        Pointer to the Device class object
    * @param cmd_name      Command name
    * @param values        Buffer storing the values to write
    * @return              device control status
    */
    control_ret_t command_set(Device * device, std::string cmd_name, uint8_t * values);

    /**
    * @brief Parse a YAML file with the list of DFU commands
     *
    * @param yaml_file_full_path    Path to the YAML file
    * @param is_verbose             Flag to indicate if verbose mode is enabled
    */
    void parse_dfu_cmds_yaml(std::string yaml_file_full_path, uint8_t is_verbose=0);

    /**
    * @brief Add a command with a given name to the list of commands
     *
    * @param yaml_info     Data read from YAML file
    * @param cmd_name      Command name
    * @param is_verbose    Flag to indicate if verbose mode is enabled
    */
    void add_command(YAML::Node yaml_info, const std::string cmd_name, uint8_t is_verbose=0);
};
