// Copyright 2022-2023 XMOS LIMITED.
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
#include <linux/limits.h>   // PATH_MAX#include <sys/stat.h>

using namespace std;


/* DFU_GETSTATUS bStatus values (Section 6.1.2, DFU Rev 1.1) */
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

/* DFU state values (Section 6.1.2, DFU Rev 1.1) */
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

/* XMOS values for control commands */

/* alt-setting for flash partition */
#define DFU_ALT_FACTORY_ID 0
#define DFU_ALT_UPGRADE_ID 1



class commandValues
{
    int id;
    int num_values;
    public:
    commandValues(int id, int num_values)
    {
        id = id;
        num_values = num_values;
    }
    int get_num_values() {return num_values;}
    int get_id() {return id;}

};
static map<string, commandValues*>CommandInfo;
static string commandList[] = {"DFU_DETACH", "DFU_DNLOAD", "DFU_UPLOAD", "DFU_GETSTATUS", "DFU_CLRSTATUS", "DFU_GETSTATE", "DFU_ABORT", "DFU_SETALTERNATE", "DFU_REBOOT", "DFU_VERSION"};

/* device control resource IDs **/
static int dfu_controller_servicer_resid = -1;
static int dfu_resid = -1;

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

const string dfu_status_to_string(int status)
{
    if (status > DFU_STATUS_errSTALLEDPKT)
        return "INVALID";
    return string(dfu_status_names[status]);
}

control_ret_t command_get(Device * device, control_resid_t res_id, std::string cmd_name, control_cmd_t cmd_id, unsigned num_values, uint8_t * values)
{
    cmd_id = cmd_id | 0x80; // setting 8th bit for read commands

    size_t data_len = num_values + 1; // one extra for the status
    uint8_t * data = new uint8_t[data_len];

    control_ret_t ret = CONTROL_SUCCESS;//device->device_get(res_id, cmd_id, data, data_len);
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
            ret = CONTROL_SUCCESS;// device->device_get(res_id, cmd_id, data, data_len);
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

control_ret_t command_set(Device * device, control_resid_t res_id, std::string cmd_name, control_cmd_t cmd_id, unsigned num_values, uint8_t * values)
{
    size_t data_len = num_values;
    uint8_t * data = new uint8_t[data_len];

    for (unsigned i = 0; i < num_values; i++)
    {
        memcpy(data + i, &values[i], 1);
    }

    control_ret_t ret = CONTROL_SUCCESS; //device->device_set(res_id, cmd_id, data, data_len);
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

opt_t options[] = {
    {"--help",                    "-h",        "display this information"                                                                       },
    {"--version",                 "-v",        "print the current version of this application",                                                 },
    {"--use",                     "-u",        "use specific hardware protocol, I2C, SPI and USB are available to use"                          },
    {"--download",                "-d",        "download upgrade image stored in the specified path, the path is relative to the working dir"   },
    {"--upload-factory",          "-uf",       "upload factory image and save it in the specified path, the path is relative to the working dir"},
    {"--upload-upgrade",          "-uu",       "upload upgrade image and save it in the specified path, the path is relative to the working dir"},
    {"--reboot",                  "-r",        "reboot device"                                                                                  },
};

size_t num_options = end(options) - begin(options);

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
    cout << "usage: xvf_host [ command | option ]" << endl
    << setw(78) << "[ -u <protocol> ] [ -cmp <path> ] [ -br ] [ command | option ]" << endl
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

control_ret_t getstatus(uint8_t &status, uint8_t &state)
{
    string command_name = "DFU_GETSTATUS";
    uint8_t values[CommandInfo[command_name]->get_num_values()];

    control_ret_t cmd_ret = command_get(NULL, dfu_resid, command_name, CommandInfo[command_name]->get_id(), CommandInfo[command_name]->get_num_values(), values);
    if (cmd_ret != CONTROL_SUCCESS) {
        cout << "Error: Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }

    uint32_t poll_timeout = ((0xff & values[3]) << 16) |
                            ((0xff & values[2]) << 8)  |
                            (0xff & values[1]);
    state = values[4];
    status = values[5];
    cout << "DFU_GETSTATUS: Status " << dfu_status_to_string(status) << ", State " << dfu_state_to_string(state) << ",Timeout (ms) " << poll_timeout << endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(poll_timeout));

    return cmd_ret;
}

control_ret_t clearStatus() {
    string command_name = "DFU_CLRSTATUS";
    control_ret_t cmd_ret = command_set(NULL, dfu_resid, command_name, CommandInfo[command_name]->get_id(), CommandInfo[command_name]->get_num_values(), NULL);
    if (cmd_ret != CONTROL_SUCCESS) {
        cout << "Error: Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }

    return cmd_ret;
}

uint32_t status_is_idle() {
    uint8_t status;
    uint8_t state;
    if (getstatus(status, state) ==  CONTROL_SUCCESS)
    {
        switch(state) {
            case DFU_STATE_appIDLE:
                return 1;
            break;
            case DFU_STATE_dfuERROR:
                clearStatus();
            break;
            default:
                // do nothing
            break;
        }
    }
    return 0;
}
control_ret_t setalternate(uint8_t alternate)
{
    string command_name = "DFU_SETALTERNATE";
    control_ret_t cmd_ret = command_set(NULL, dfu_resid, command_name, CommandInfo[command_name]->get_id(), CommandInfo[command_name]->get_num_values(), NULL);
    if (cmd_ret != CONTROL_SUCCESS) {
        cout << "Error: Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }

    return cmd_ret;
}
control_ret_t download_operation(const string image_path)
{
    cout << "Download upgrade image " << image_path << endl;
    ifstream rf(image_path, ios::out | ios::binary);
    if(!rf) {
        cout << "Cannot open file!" << endl;
        return CONTROL_ERROR;
    }
    uint8_t status;
    uint8_t state;
    size_t file_size = rf.tellg();
    uint32_t total_bytes = 0;
    control_ret_t cmd_ret;
    uint8_t is_state_not_idle = 1;
    uint8_t is_state_not_error = 1;
    while (total_bytes <= file_size) {
        string command_name = "DFU_DNLOAD";
        uint8_t num_values = CommandInfo[command_name]->get_num_values();
        uint8_t * values = new uint8_t[num_values];
        rf.read((char*) values, num_values);
        cmd_ret = command_set(NULL, dfu_resid, command_name, CommandInfo[command_name]->get_id(), num_values, values);
        if (cmd_ret != CONTROL_SUCCESS) {
            cout << "Error: Command " << command_name << " returned error " << cmd_ret << endl;
            return cmd_ret;
        }
        while (is_state_not_idle && is_state_not_error)
        {
            if (getstatus(status, state) ==  CONTROL_SUCCESS)
            {
                switch(state) {
                    case DFU_STATE_dfuDNLOAD_IDLE:
                        is_state_not_idle = 0;
                    break;
                    case DFU_STATE_dfuERROR:
                        clearStatus();
                        is_state_not_error = 0;
                    break;
                    default:
                        continue;
                    break;
                }
            }
        }
        total_bytes += num_values;
    }
    rf.close();
    if(!rf.good()) {
        cout << "Error: Reading from file " << image_path << " failed" << endl;
        return CONTROL_ERROR;
    }
    // Send empty download message. TODO: Check how to indicate the zero payload length
    string command_name = "DFU_DNLOAD";
    cmd_ret = command_set(NULL, dfu_resid, command_name, CommandInfo[command_name]->get_id(), CommandInfo[command_name]->get_num_values(), 0);
    if (cmd_ret != CONTROL_SUCCESS) {
        cout << "Error: Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }
    while (is_state_not_idle)
    {
        if (getstatus(status, state) ==  CONTROL_SUCCESS)
        {
            switch(state) {
                case DFU_STATE_dfuIDLE:
                    is_state_not_idle = 0;
                break;
                case DFU_STATE_dfuMANIFEST_SYNC:
                case DFU_STATE_dfuMANIFEST:
                    continue;
                break;
                default:
                    cout << "Error: Invalid state: " << dfu_state_to_string(state) << endl;
                    return CONTROL_ERROR;
                break;
            }
        }
    }
    return CONTROL_SUCCESS;
}

control_ret_t reboot_operation()
{
    cout << "Reboot device" << endl;
    string command_name = "DFU_DETACH";
    control_ret_t cmd_ret = command_set(NULL, dfu_resid, command_name, CommandInfo[command_name]->get_id(), CommandInfo[command_name]->get_num_values(), NULL);
    if (cmd_ret != CONTROL_SUCCESS) {
        cout << "Error: Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }
    return CONTROL_SUCCESS;
}

control_ret_t upload_operation(const string image_path)
{
    cout << "Upload image to " << image_path << endl;
    ofstream wf(image_path, ios::out | ios::binary);
    if(!wf) {
        cout << "Cannot open file!" << endl;
        return CONTROL_ERROR;
    }
    while (1) {
        string command_name = "DFU_UPLOAD";
        uint8_t num_values = CommandInfo[command_name]->get_num_values();
        uint8_t values[num_values];

        control_ret_t cmd_ret = command_get(NULL, dfu_resid, command_name, CommandInfo[command_name]->get_id(), num_values, values);
        if (cmd_ret != CONTROL_SUCCESS) {
            cout << "Error: Command " << command_name << " returned error " << cmd_ret << endl;
            return cmd_ret;
        }
        wf.write((const char *) values, num_values);
    }

    wf.close();
    if(!wf.good()) {
        cout << "Error: Writing to file " << image_path << " failed" << endl;
        return CONTROL_ERROR;
    }
    return CONTROL_SUCCESS;
}

int file_path_exists(char ** argv, const uint32_t arg_indx, const uint32_t argc, string& path)
{
    if (arg_indx >= argc)
    {
        cout << "Error: missing file path" << endl;

        return -1;
    }
    path = convert_to_abs_path(argv[arg_indx]);
    struct stat buffer;
    if (stat(path.c_str(), &buffer) == 0)
    {
        return 1;
    } else {
        return 0;
    }
}

void add_command(YAML::Node yaml_info, const string command_name)
{
    for (const auto& command : yaml_info)
    {
        if (command["cmd"].as<string>().find(command_name) != string::npos)
        {
            int cmd_id = command["index"].as<int>();
            int cmd_num_values = command["number_of_values"].as<int>();
            commandValues* c = new commandValues(cmd_id, cmd_num_values);
            CommandInfo[command_name] = c;
            cout << "Added command " << command_name << " with ID " << cmd_id << " and number of values " << cmd_num_values << endl;
            return;
        }
    }
    cerr << "Warning: command " << command_name << " not found on yaml file" << endl;
    exit(1);
}

void parse_dfu_cmds_yaml(string yaml_file_full_path)
{
    // Load YAML file
    YAML::Node config = YAML::LoadFile(yaml_file_full_path);
    // string dfu_detach_cmd_name = config["DFU_CONTROLLER_SERVICER_RESID (0xF0)"]["dedicated_commands"][0]["cmd"].as<string>();
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
            cout << "DFU_CONTROLLER_SERVICER_RESID is " << resource_id << endl;
            dfu_controller_servicer_resid = resource_id;
        }
        if (resource_id_string.find("DFU_RESID") != string::npos)
        {
            cout << "DFU_RESID is " << resource_id << endl;
            dfu_resid = resource_id;
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
        cerr << "Error: DFU_CONTROLLER_SERVICER_RESID not set in YAML file" << endl;
        exit(1);
    }
    if (dfu_resid == -1)
    {
        cerr << "Error: DFU_RESID not set in YAML file" << endl;
        exit(1);
    }
}

string get_file_path(const string rel_path)
{

    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count == -1)
    {
        cerr << "Could not read /proc/self/exe into " << PATH_MAX << " string" << endl;
        exit(1);
    }
    path[count] = '\0'; // readlink doesn't always add NULL for some reason

    string full_path = path;
    size_t found = full_path.find_last_of("/\\"); // works for both unix and windows
    string dir_path_str = full_path.substr(0, found);
    string file_path_str = dir_path_str + "/" + rel_path;

    return file_path_str;
}

int main(int argc, char ** argv)
{
    if(argc == 1)
    {
        cout << "Use --help to get the list of options for this application." << endl
        << "Or use --list-commands to print the list of commands and their info." << endl;
        return 0;
    }

    // Load YAML file
    YAML::Node config = YAML::LoadFile(get_file_path("src/transport_config.yaml"));

    // Read I2C parameters from YAML file
    int* i2c_info = new int[1];
    int* spi_info = new int[2];

    i2c_info[0] = config["I2C_ADDRESS"].as<int>();
    spi_info[0] = config["SPI_MODE"].as<int>();
    spi_info[1] = 1024;

    parse_dfu_cmds_yaml(get_file_path("src/dfu_cmds.yaml"));
    string device_dl_name = get_device_lib_name(&argc, argv, options, num_options);
    int * device_init_info = NULL;
    if(device_dl_name == device_i2c_dl_name)
    {
        device_init_info = i2c_info;
    } else if(device_dl_name == device_spi_dl_name)
    {
        device_init_info = spi_info;
    }

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
        else if (opt->long_name == "--version")
        {
            cout << current_host_app_version << endl;
            return 0;
        }
    }
    #if 0
    string device_dl_path = get_dynamic_lib_path(device_dl_name);
    dl_handle_t device_handle = get_dynamic_lib(device_dl_path);

    device_fptr make_dev = get_device_fptr(device_handle);
    Device * device = make_dev(device_init_info);

    control_ret_t ret = device->device_init();
    if (ret != CONTROL_SUCCESS)
    {
        cerr << "Could not connect to the device" << endl;
        exit(ret);
    }
    #endif

    int arg_indx = cmd_indx + 1;
    next_cmd = argv[cmd_indx];
    if(next_cmd[0] == '-')
    {
        if (opt->long_name == "--upload-factory")
        {
            setalternate(DFU_ALT_FACTORY_ID);
        } else {
            setalternate(DFU_ALT_UPGRADE_ID);
        }
        if (!status_is_idle()) {
            return -1;
        }
        opt = option_lookup(next_cmd, options, num_options);
        if (opt->long_name == "--download")
        {
            string image_path = "";
            if (file_path_exists(argv, arg_indx, argc, image_path)) {
                download_operation(image_path);
            } else {
                cout << "Error: File at path \'" << argv[arg_indx] << "\' not found" << endl;
                return -1;
            }
        }
        if (opt->long_name == "--reboot")
        {
            reboot_operation();
        }
        if (opt->long_name == "--upload-factory" || opt->long_name == "--upload-upgrade")
        {
            string image_path = "";
            if (!file_path_exists(argv, arg_indx, argc, image_path)) {
                upload_operation(image_path);
            } else {
                cout << "Error: File at path \'" << argv[arg_indx] << "\' already exists" << endl;
                return -1;
            }
        }
    }

    // Program should NEVER get to this point
    cout << "Host application behaved unexpectedly, please report this issue" << endl;
    return -1;
}
