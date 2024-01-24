// Copyright 2023-2024 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include <fstream>
#include <iostream>
#include "utils.hpp"
#include <iomanip>
#include <chrono>
#include <thread>
#include <map>
#include <sys/stat.h>
#include <yaml-cpp/yaml.h>
#include <unistd.h>         // readlink
#include <sys/ioctl.h>      // ioctl

using namespace std;

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

/** @brief Size of transfer buffer for upload and download operations */
#define DFU_DATA_BUFFER_SIZE 64

/**
 * @brief Dictionaries storing command IDs and payload lenghts of the DFU commands
-* @note The values are read from the DFU yaml file
 **/
static map<string, int>CommandIDs;
static map<string, int>CommandLengths;

/** @brief List of supported DFU commands */
static string commandList[] =
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

/** @brief Resource ID of DFU controller servicer
 * @note The value is read from the DFU yaml file
 **/
static int dfu_controller_servicer_resid = -1;

/** @brief Convert DFU state value into a string */
const string dfu_state_to_string( int state )
{
    const char * message;

    switch (state) {
        case DFU_STATE_appIDLE:
            message = "appIDLE";
            break;
        case DFU_STATE_appDETACH:
            message = "appDETACH";
            break;
        case DFU_STATE_dfuIDLE:
            message = "dfuIDLE";
            break;
        case DFU_STATE_dfuDNLOAD_SYNC:
            message = "dfuDNLOAD-SYNC";
            break;
        case DFU_STATE_dfuDNBUSY:
            message = "dfuDNBUSY";
            break;
        case DFU_STATE_dfuDNLOAD_IDLE:
            message = "dfuDNLOAD-IDLE";
            break;
        case DFU_STATE_dfuMANIFEST_SYNC:
            message = "dfuMANIFEST-SYNC";
            break;
        case DFU_STATE_dfuMANIFEST:
            message = "dfuMANIFEST";
            break;
        case DFU_STATE_dfuMANIFEST_WAIT_RST:
            message = "dfuMANIFEST-WAIT-RESET";
            break;
        case DFU_STATE_dfuUPLOAD_IDLE:
            message = "dfuUPLOAD-IDLE";
            break;
        case DFU_STATE_dfuERROR:
            message = "dfuERROR";
            break;
        default:
            message = NULL;
            break;
    }

    return string(message);
}

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

/**
 * @brief Check if file exists
 *
 * @param path          path of the given file
 *
 * @return              1 if file is found, 0 otherwise
 */
uint32_t is_file_found(string path)
{
    struct stat buffer;
    if (stat(path.c_str(), &buffer) == 0)
    {
        return 1;
    } else {
        return 0;
    }
}

/** @brief Convert DFU status value into a string */
const string dfu_status_to_string(int status)
{
    if (status > DFU_STATUS_errSTALLEDPKT)
        return "INVALID";
    return string(dfu_status_names[status]);
}

/**
 * @brief Executes a single get command
 *
 * @param device        Pointer to the Device class object
 * @param res_id        Command resource ID
 * @param cmd_name      Command Name
 * @param cmd_id        Command ID
 * @param num_values    Number of values the command reads
 * @param values        Buffer storing the read values
 *
 * @return              device control status
 */
control_ret_t command_get(Device * device, control_resid_t res_id, string cmd_name, control_cmd_t cmd_id, unsigned num_values, uint8_t * values)
{
    cmd_id = cmd_id | 0x80; // setting 8th bit for read commands

    size_t data_len = num_values + 1; // one extra for the status
    uint8_t * data = new uint8_t[data_len];

    control_ret_t ret = device->device_get(res_id, cmd_id, data, data_len);
    int read_attempts = 1;

    while(1)
    {
        if(read_attempts == 1000)
        {
            cerr << "Resource could not respond to the " << cmd_name << " read command."
            << endl << "Check the audio loop is active." << endl;
            exit(HOST_APP_ERROR);
        }
        if(data[0] == CONTROL_SUCCESS)
        {
            for (unsigned i = 0; i < num_values; i++)
            {
                memcpy(&values[i], &data[1] + i, 1);
            }
            break;
        }
        else if(data[0] == SERVICER_COMMAND_RETRY)
        {
            ret = device->device_get(res_id, cmd_id, data, data_len);
            read_attempts++;
        }
        else
        {
            check_cmd_error(cmd_name, "read", static_cast<control_ret_t>(data[0]));
        }
    }

    delete []data;
    check_cmd_error(cmd_name, "read", ret);
    return ret;
}

/**
 * @brief Executes a single set command
 *
 * @param device        Pointer to the Device class object
 * @param res_id        Command resource ID
 * @param cmd_name      Command Name
 * @param cmd_id        Command ID
 * @param num_values    Number of values the command writes
 * @param values        Buffer storing the values to write
 *
 * @return              device control status
 */
control_ret_t command_set(Device * device, control_resid_t res_id, string cmd_name, control_cmd_t cmd_id, unsigned num_values, uint8_t * values)
{
    size_t data_len = num_values;
    uint8_t * data = new uint8_t[data_len];

    for (unsigned i = 0; i < num_values; i++)
    {
        memcpy(data + i, &values[i], 1);
    }

    control_ret_t ret = device->device_set(res_id, cmd_id, data, data_len);
    int write_attempts = 1;

    while(1)
    {
        if(write_attempts == 1000)
        {
            cerr << "Resource could not respond to the " << cmd_name << " write command."
            << endl << "Check the audio loop is active." << endl;
            exit(HOST_APP_ERROR);
        }
        if(ret == CONTROL_SUCCESS)
        {
            break;
        }
        else if(ret == SERVICER_COMMAND_RETRY)
        {
            ret = device->device_set(res_id, cmd_id, data, data_len);
            write_attempts++;
        }
        else
        {
            check_cmd_error(cmd_name, "write", ret);
        }
    }

    delete []data;
    check_cmd_error(cmd_name, "write", ret);
    return ret;
}

/** @brief List of supported CLI options */
opt_t options[] = {
    {"--help",                    "-h",        "display this information"                                                                       },
    {"--app-version",             "-av",       "print the version of this application",                                                 },
    {"--use",                     "-u",        "use specific hardware protocol, I2C, SPI and USB are available to use"                          },
    {"--verbose",                 "-vvv",      "enable debug prints"                                                                            },
    {"--version",                 "-v",        "read the version on the device",                                                                },
    {"--download",                "-d",        "download upgrade image stored in the specified path, the path is relative to the working dir"   },
    {"--upload-factory",          "-uf",       "upload factory image and save it in the specified path, the path is relative to the working dir"},
    {"--upload-upgrade",          "-uu",       "upload upgrade image and save it in the specified path, the path is relative to the working dir"},
    {"--reboot",                  "-r",        "reboot device"                                                                                  },
};
size_t num_options = end(options) - begin(options);

static uint32_t verbose_mode = 0;

/** @brief Print DFU host application help menu */
control_ret_t print_help_menu()
{
    size_t longest_short_opt = 0;
    size_t longest_long_opt = 0;
    for(opt_t opt : options)
    {
        size_t short_len = opt.short_name.length();
        size_t long_len = opt.long_name.length();
        longest_short_opt = (short_len > longest_short_opt) ? short_len : longest_short_opt;
        longest_long_opt = (long_len > longest_long_opt) ? long_len : longest_long_opt;
    }
    size_t long_opt_offset = longest_short_opt + 5;
    size_t info_offset = long_opt_offset + longest_long_opt + 4;
    // Getting current terminal width here to set the cout line limit
    const size_t hard_stop = get_term_width();

    // Please avoid lines which have more than 80 characters
    cout << "usage: xvf_dfu [ -u <protocol> ] [ --verbose ] command" << endl
    << endl << "Current application version is " << current_host_app_version << "."
    << endl << "You can use --use or -u option to specify protocol you want to use."
    << endl << "Default control protocol is I2C."
    << endl << endl << "Options:" << endl;
    for(opt_t opt : options)
    {
        size_t short_len = opt.short_name.length();
        size_t long_len = opt.long_name.length();
        size_t info_len = opt.info.length();
        size_t first_word_len = opt.info.find_first_of(' ');
        int first_space = long_opt_offset - short_len + long_len;
        int second_space = info_offset - long_len - long_opt_offset + first_word_len;
        int num_spaces = 2; // adding two black spaces at the beginning to make it look nicer

        cout << setw(num_spaces) << " " << opt.short_name << setw(first_space)
        << opt.long_name << setw(second_space);

        stringstream ss(opt.info);
        string word;
        size_t curr_pos = info_offset + num_spaces;
        while(ss >> word)
        {
            size_t word_len = word.length();
            size_t future_pos = curr_pos + word_len + 1;
            if(future_pos >= hard_stop)
            {
                cout << endl << setw(info_offset + word_len + num_spaces) << word << " ";
                curr_pos = info_offset + word_len + num_spaces + 1;
            }
            else
            {
                cout << word << " ";
                curr_pos = future_pos;
            }
        }
        cout << endl << endl;
    }
    return CONTROL_SUCCESS;
}


/**
 * @brief Executes a GETSTATUS request
 *
 * @param device        Pointer to the Device class object
 * @param status        Status read from the device
 * @param state         State read from the device
 *
 * @return              device control status
 */
control_ret_t getstatus(Device * device, uint8_t &status, uint8_t &state)
{

    const string command_name = "DFU_GETSTATUS";
    uint8_t values[CommandLengths[command_name]];
    if (verbose_mode) {
        cout << "Send DFU_GETSTATUS message" << endl;
    }
    control_ret_t cmd_ret = command_get(device, dfu_controller_servicer_resid, command_name, CommandIDs[command_name], CommandLengths[command_name], values);
    if (cmd_ret != CONTROL_SUCCESS) {
        cerr << "Error: Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }
    status = values[0];
    uint32_t poll_timeout = ((0xff & values[3]) << 16) |
                            ((0xff & values[2]) << 8)  |
                            (0xff & values[1]);
    state = values[4];
    if (verbose_mode) {
        cout << "DFU_GETSTATUS: Status " << dfu_status_to_string(status) << ", State " << dfu_state_to_string(state) << ", Timeout (ms) " << poll_timeout << endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(poll_timeout));

    return cmd_ret;
}

/**
 * @brief Executes a CLRSTATUS request
 *
 * @return              device control status
 */
control_ret_t clearStatus(Device * device) {
    control_ret_t cmd_ret = CONTROL_SUCCESS;
    string command_name = "DFU_CLRSTATUS";
    uint8_t num_values = CommandLengths[command_name];
    uint8_t values[num_values];
    if (verbose_mode) {
        cout << "Send DFU_CLRSTATUS message" << endl;
    }
    cmd_ret = command_set(device, dfu_controller_servicer_resid, command_name, CommandIDs[command_name], CommandLengths[command_name], values);
    if (cmd_ret != CONTROL_SUCCESS) {
        cerr << "Error: Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }

    return cmd_ret;
}

/**
 * @brief Checks if device is in dfuIDLE state
 *
 * @param device        Pointer to the Device class object
 *
 * @return              1 if it the state is dfuIDLE, 0 otherwise
 */
uint32_t status_is_idle(Device * device) {
    uint8_t status;
    uint8_t state;

    if (getstatus(device, status, state) ==  CONTROL_SUCCESS)
    {
        switch(state) {
            case DFU_STATE_dfuERROR:
                clearStatus(device);
                exit(HOST_APP_ERROR);
            break;
            case DFU_STATE_dfuIDLE:
                return 1;
            default:
                // do nothing
            break;
        }
    }
    return 0;
}

/**
 * @brief Executes a SETALTERNATE request
 *
 * @param device        Pointer to the Device class object
 * @param alternate     Alt setting to use
 *
 * @return              device control status
 */
control_ret_t setalternate(Device * device, uint8_t alternate)
{
    control_ret_t cmd_ret = CONTROL_SUCCESS;
    string command_name = "DFU_SETALTERNATE";
    uint8_t num_values = CommandLengths[command_name];
    uint8_t values[num_values];
    values[0] = alternate;
    if (verbose_mode) {
        cout << "Send DFU_SETALTERNATE message with value " << unsigned(alternate) << endl;
    }
    cmd_ret = command_set(device, dfu_controller_servicer_resid, command_name, CommandIDs[command_name], CommandLengths[command_name], values);
    if (cmd_ret != CONTROL_SUCCESS) {
        cerr << "Error: Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }

    return cmd_ret;
}

/**
 * @brief Executes a download operation
 *
 * @param device        Pointer to the Device class object
 * @param image_path    Path to the image to download to the device
 *
 * @return              device control status
 */
control_ret_t download_operation(Device * device, const string image_path)
{
    cout << "Download upgrade image " << image_path << endl;
    ifstream rf(image_path, ios::in | ios::binary);
    if(!rf) {
        cout << "Cannot open file!" << endl;
        return CONTROL_ERROR;
    }
    uint8_t status;
    uint8_t state;
    // Read file size
    rf.seekg (0, ios::end);
    size_t file_size = rf.tellg();
    // Set position to the beginning of the file
    rf.seekg (0, ios::beg);
    uint32_t total_bytes = 0;
    control_ret_t cmd_ret = CONTROL_SUCCESS;
    uint8_t is_state_not_dn_idle = 1;
    string command_name = "DFU_DNLOAD";
    uint8_t num_values = CommandLengths[command_name];
    uint8_t * values = new uint8_t[num_values];
    while (rf.good()) {

        values[0] = DFU_DATA_BUFFER_SIZE;
        rf.read((char*) &values[1], DFU_DATA_BUFFER_SIZE);
        is_state_not_dn_idle = 1;
        if (verbose_mode) {
            cout << "Send DFU_DNLOAD message with " << DFU_DATA_BUFFER_SIZE << " bytes" << endl;
        }
        cmd_ret = command_set(device, dfu_controller_servicer_resid, command_name, CommandIDs[command_name], num_values, values);
        if (cmd_ret != CONTROL_SUCCESS) {
            cerr << "Error: Command " << command_name << " returned error " << cmd_ret << endl;
            return cmd_ret;
        }
        // Wait till device is in state dfuDNLOAD_IDLE
        while (is_state_not_dn_idle)
        {
            if (getstatus(device, status, state) ==  CONTROL_SUCCESS)
            {
                switch(state) {
                    case DFU_STATE_dfuDNLOAD_IDLE:
                        is_state_not_dn_idle = 0;
                    break;
                    case DFU_STATE_dfuERROR:
                        clearStatus(device);
                        exit(HOST_APP_ERROR);
                    break;
                    default:
                        continue;
                    break;
                }
            }
        }
        cout << setprecision(2) << fixed;
        cout << "\rDownloaded " << (float) total_bytes / file_size * 100 << "% of the image" << std::flush;
        if (verbose_mode) {
            cout << endl;
        }
        total_bytes += DFU_DATA_BUFFER_SIZE;
    }
    cout << endl;


    rf.close();

    // Send empty download message
    cout << "Download completed. Send DFU_DNLOAD message with size zero" << endl;
    memset(values, 0, CommandLengths[command_name]);
    cmd_ret = command_set(device, dfu_controller_servicer_resid, command_name, CommandIDs[command_name], CommandLengths[command_name], values);
    if (cmd_ret != CONTROL_SUCCESS) {
        cerr << "Error: Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }

    // Wait till device is in state dfuIDLE
    is_state_not_dn_idle = 1;
    while (!status_is_idle(device)) { }
    return CONTROL_SUCCESS;
}

/**
 * @brief Executes a reboot operation
 *
 * @return              device control status
 */
control_ret_t reboot_operation(Device * device)
{
    cout << "Reboot device" << endl;
    control_ret_t cmd_ret = CONTROL_SUCCESS;

    string command_name = "DFU_DETACH";
    uint8_t num_values = CommandLengths[command_name];
    uint8_t values[num_values];
    if (verbose_mode) {
        cout << "Send DFU_DETACH message" << endl;
    }
    cmd_ret = command_set(device, dfu_controller_servicer_resid, command_name, CommandIDs[command_name], CommandLengths[command_name], values);
    if (cmd_ret != CONTROL_SUCCESS) {
        cerr << "Error: Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }

    return cmd_ret;
}

/**
 * @brief Executes a download operation
 *
 * @param device        Pointer to the Device class object
 * @param image_path    Path to the image to upload from the device
 *
 * @return              device control status
 */
control_ret_t upload_operation(Device * device, const string image_path)
{
    cout << "Uploading image to " << image_path << endl;
    ofstream wf(image_path, ios::out | ios::binary);

    string command_name = "DFU_UPLOAD";
    uint8_t num_values = CommandLengths[command_name];
    uint8_t values[num_values];
    uint32_t transfer_block_num = 0;
    uint32_t transfer_ongoing = 1;
    uint32_t transfer_block_size = 0;
    if(!wf) {
        cout << "Cannot open file!" << endl;
        return CONTROL_ERROR;
    }
    while (transfer_ongoing) {
        if (verbose_mode) {
            cout << "Send DFU_UPLOAD message" << endl;
        }
        control_ret_t cmd_ret = command_get(device, dfu_controller_servicer_resid, command_name, CommandIDs[command_name], num_values, values);
        if (cmd_ret != CONTROL_SUCCESS) {
            cerr << "Error: Command " << command_name << " returned error " << cmd_ret << endl;
            return cmd_ret;
        }
        transfer_block_size = values[0];
        if (transfer_block_size) {
            cout << "\rUploaded " << transfer_block_num << " blocks of " << DFU_DATA_BUFFER_SIZE << " bytes" << std::flush;
            wf.write((const char *) &values[1], values[0]);
        }
        if (verbose_mode) {
            cout << endl;
        }
        // Wait till we receive a DFU_UPLOAD with no data
        if (transfer_block_size < DFU_DATA_BUFFER_SIZE) {
            cout << "Received transport block with size " << transfer_block_size << " (smaller than "<< DFU_DATA_BUFFER_SIZE << "): upload complete" << endl;
            transfer_ongoing = 0;
        }
        transfer_block_num++;

    }

    wf.close();
    if(!wf.good()) {
        cerr << "Error: Writing to file " << image_path << " failed" << endl;
        return CONTROL_ERROR;
    }
    return CONTROL_SUCCESS;
}

/**
 * @brief Print version of the device
 *
 * @param device        Pointer to the Device class object
 *
 * @return              device control status
 */
control_ret_t getversion(Device * device)
{

    const string command_name = "DFU_GETVERSION";
    uint8_t values[CommandLengths[command_name]];
    if (verbose_mode) {
        cout << "Send DFU_GETVERSION message" << endl;
    }
    control_ret_t cmd_ret = command_get(device, dfu_controller_servicer_resid, command_name, CommandIDs[command_name], CommandLengths[command_name], values);
    if (cmd_ret != CONTROL_SUCCESS) {
        cerr << "Error: Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }
    cout << "DFU_GETVERSION: ";
    for (int i=0; i<CommandLengths[command_name]; i++) {
        cout << unsigned(values[i]) << " ";
    }
    cout << endl;

    return cmd_ret;
}

/**
 * @brief Add a DFU command using the info found in the YAML file
 *
 * @param yaml_info     Information read from the YAML file
 * @param command_name  DFU command name to be added
 */
void add_command(YAML::Node yaml_info, const string command_name)
{
    for (const auto& command : yaml_info)
    {
        if (command["cmd"].as<string>().find(command_name) != string::npos)
        {
            int cmd_id = command["index"].as<int>();
            int cmd_num_values = command["number_of_values"].as<int>();
            CommandIDs[command_name] = cmd_id;
            CommandLengths[command_name] = cmd_num_values;
            if (verbose_mode) {
                cout << "Added command " << command_name << " with ID " << CommandIDs[command_name] << " and number of values " << cmd_num_values << endl;
            }
            return;
        }
    }
    cerr << "Error: command " << command_name << " not found in yaml file" << endl;
    exit(HOST_APP_ERROR);
}

/**
 * @brief Parse YAML file with DFU control commands
 *
 * @param yaml_file_full_path     Path to the YAML file
 */
void parse_dfu_cmds_yaml(string yaml_file_full_path)
{
    // Load YAML file
    YAML::Node config = YAML::LoadFile(yaml_file_full_path);
    YAML::Node resource_ids = config;

    for (const auto& node : config)
    {
        string resource_id_string = node.first.as<string>();
        size_t startPos = resource_id_string.find('(');
        size_t endPos = resource_id_string.find(')', startPos);
        int resource_id = -1;
        if (startPos != string::npos and endPos != string::npos)
        {
            resource_id = stoi(resource_id_string.substr(startPos + 1, endPos - startPos - 1), 0, 16);
        }
        if (resource_id_string.find("DFU_CONTROLLER_SERVICER_RESID") != string::npos)
        {
            if (verbose_mode) {
                cout << "DFU_CONTROLLER_SERVICER_RESID is " << resource_id << endl;
            }
            dfu_controller_servicer_resid = resource_id;
        }
        YAML::Node dedicated_commands = node.second["dedicated_commands"];
        if (dedicated_commands.IsSequence())
        {
            for (const auto& command_name : commandList)
            {
                add_command(dedicated_commands, command_name);
            }
        }
    }
    if (dfu_controller_servicer_resid == -1)
    {
        cerr << "Error: DFU_CONTROLLER_SERVICER_RESID not set. Check the YAML file: " << yaml_file_full_path << endl;
        exit(HOST_APP_ERROR);
    }
}

int main(int argc, char ** argv)
{
    if(argc == 1)
    {
        cout << "Use --help to get the list of options for this application." << endl
        << "Or use --list-commands to print the list of commands and their info." << endl;
        return 0;
    }
    YAML::Node config;
    int* i2c_info = new int[1];
    int* spi_info = new int[2];

    // Load YAML file with transport settings
    string file_name = "transport_config.yaml";
    if (is_file_found(file_name)) {
        config = YAML::LoadFile("transport_config.yaml");
        // Read I2C parameters from YAML file
        i2c_info[0] = config["I2C_ADDRESS"].as<int>();
        spi_info[0] = config["SPI_MODE"].as<int>();
        spi_info[1] = 1024;
    } else {
        cerr << "Error: File \'" << file_name << "\' not found" << endl;
        exit(HOST_APP_ERROR);
    }

    // Load YAML file with DFU commands info
    file_name = "dfu_cmds.yaml";
    if (is_file_found(file_name)) {
        parse_dfu_cmds_yaml("dfu_cmds.yaml");
    } else {
        cerr << "Error: File \'" << file_name << "\' not found" << endl;
        exit(HOST_APP_ERROR);
    }

    // Check first CLI options which don't require access to the device
    const opt_t * opt = nullptr;
    int cmd_indx = 1;
    string next_cmd = argv[cmd_indx];
    if(next_cmd[0] == '-')
    {
        opt = option_lookup(next_cmd, options, num_options);
        if (opt->long_name == "--help")
        {
            return print_help_menu();
        }
        else if (opt->long_name == "--app-version")
        {
            cout << current_host_app_version << endl;
            return 0;
        }
    }

    // Load dynamic library with transport drivers
    string device_dl_name = get_device_lib_name(&argc, argv, options, num_options);
    int * device_init_info = NULL;
    if(device_dl_name == device_i2c_dl_name)
    {
        device_init_info = i2c_info;
    } else if(device_dl_name == device_spi_dl_name)
    {
        device_init_info = spi_info;
    }
    string device_dl_path = get_dynamic_lib_path(device_dl_name);
    dl_handle_t device_handle = get_dynamic_lib(device_dl_path);

    device_fptr make_dev = get_device_fptr(device_handle);
    Device * device = make_dev(device_init_info);

    control_ret_t ret = device->device_init();
    if (ret != CONTROL_SUCCESS)
    {
        cerr << "Could not connect to the device: error " << ret << endl;
        exit(HOST_APP_ERROR);
    }

    // Check CLI options which require access to the device
    next_cmd = argv[cmd_indx];
    if(next_cmd[0] == '-')
    {
        opt = option_lookup(next_cmd, options, num_options);
        // Check if verbose mode is set
        if (opt->long_name == "--verbose")
        {
            cout << "Verbose mode enabled" << endl;
            verbose_mode = 1;
            cmd_indx++;
            next_cmd = argv[cmd_indx];
        }
        int arg_indx = cmd_indx + 1;
        opt = option_lookup(next_cmd, options, num_options);

        if (opt->long_name == "--version")
        {
            getversion(device);
            exit(0);
        }
        // Set appropriate ALT-SETTING
        if (opt->long_name == "--upload-factory")
        {
            setalternate(device, DFU_ALT_FACTORY_ID);
        } else {
            setalternate(device, DFU_ALT_UPGRADE_ID);
        }
        if (!status_is_idle(device)) {
            return -1;
        }

        if (opt->long_name == "--download")
        {
            string image_path = "";
            if (arg_indx >= argc)
            {
                cerr << "Error: missing file path" << endl;
                exit(HOST_APP_ERROR);
            } else {
                image_path = convert_to_abs_path(argv[arg_indx]);
            }
            if (is_file_found(image_path)) {
                download_operation(device, image_path);
            } else {
                cerr << "Error: File at path \'" << argv[arg_indx] << "\' not found" << endl;
                return -1;
            }
        }
        if (opt->long_name == "--reboot")
        {
            reboot_operation(device);
        }
        if (opt->long_name == "--upload-factory" || opt->long_name == "--upload-upgrade")
        {
            string image_path = "";
            if (arg_indx >= argc)
            {
                cerr << "Error: missing file path" << endl;
                exit(HOST_APP_ERROR);
            } else {
                image_path = convert_to_abs_path(argv[arg_indx]);
            }
            if (!is_file_found(image_path)) {
                upload_operation(device, image_path);
            } else {
                cerr << "Error: File at path \'" << argv[arg_indx] << "\' already exists" << endl;
                return -1;
            }
        }
    }
}
