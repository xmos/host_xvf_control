// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include <fstream>
#include <iostream>
#include "utils.hpp"
#include <iomanip>
#include <chrono>
#include <thread>

#include <sys/stat.h>

using namespace std;

static int i2c_info = 0x2C;    // I2C slave address

static int spi_info[2] = {
    0,                  // SPI_MODE
    1024                // clock divider
};
#define ALT_FACTORY_ID 0
#define ALT_UPGRADE_ID 1


/* DFU_GETSTATUS bStatus values (Section 6.1.2, DFU Rev 1.1) */
#define DFU_STATUS_OK			0x00
#define DFU_STATUS_errTARGET		0x01
#define DFU_STATUS_errFILE		0x02
#define DFU_STATUS_errWRITE		0x03
#define DFU_STATUS_errERASE		0x04
#define DFU_STATUS_errCHECK_ERASED	0x05
#define DFU_STATUS_errPROG		0x06
#define DFU_STATUS_errVERIFY		0x07
#define DFU_STATUS_errADDRESS		0x08
#define DFU_STATUS_errNOTDONE		0x09
#define DFU_STATUS_errFIRMWARE		0x0a
#define DFU_STATUS_errVENDOR		0x0b
#define DFU_STATUS_errUSBR		0x0c
#define DFU_STATUS_errPOR		0x0d
#define DFU_STATUS_errUNKNOWN		0x0e
#define DFU_STATUS_errSTALLEDPKT	0x0f

enum dfu_state {
	DFU_STATE_appIDLE		= 0,
	DFU_STATE_appDETACH		= 1,
	DFU_STATE_dfuIDLE		= 2,
	DFU_STATE_dfuDNLOAD_SYNC	= 3,
	DFU_STATE_dfuDNBUSY		= 4,
	DFU_STATE_dfuDNLOAD_IDLE	= 5,
	DFU_STATE_dfuMANIFEST_SYNC	= 6,
	DFU_STATE_dfuMANIFEST		= 7,
	DFU_STATE_dfuMANIFEST_WAIT_RST	= 8,
	DFU_STATE_dfuUPLOAD_IDLE	= 9,
	DFU_STATE_dfuERROR		= 10
};

#define DFU_CONTROLLER_SERVICER_RESID 18
#define DFU_DETACH_CMD_ID 0x00
#define DFU_DETACH_CMD_NAME "DFU_DETACH"
#define DFU_DETACH_NUM_VALUES 61
#define DFU_DOWNLOAD_CMD_ID 0x01
#define DFU_DOWNLOAD_CMD_NAME "DFU_DOWNLOAD"
#define DFU_DOWNLOAD_NUM_VALUES 61
#define DFU_UPLOAD_CMD_ID 0x02
#define DFU_UPLOAD_CMD_NAME "DFU_UPLOAD"
#define DFU_UPLOAD_NUM_VALUES 61
#define DFU_GETSTATUS_CMD_ID 0x03
#define DFU_GETSTATUS_CMD_NAME "DFU_GETSTATUS"
#define DFU_GETSTATUS_NUM_VALUES 5
#define DFU_CLRSTATUS_CMD_ID 0x04
#define DFU_CLRSTATUS_CMD_NAME "DFU_CLRSTATUS"
#define DFU_CLRSTATUS_NUM_VALUES 0
#define DFU_GETSTATE_CMD_ID 0x05
#define DFU_GETSTATE_CMD_NAME "DFU_GETSTATE"
#define DFU_GETSTATE_NUM_VALUES 1
#define DFU_ABORT_CMD_ID 0x06
#define DFU_ABORT_CMD_NAME "DFU_ABORT"
#define DFU_ABORT_NUM_VALUES 0
#define DFU_SETALTERNATE_CMD_ID 0x40
#define DFU_SETALTERNATE_CMD_NAME "DFU_SETALTERNATE"
#define DFU_SETALTERNATE_NUM_VALUES 1
#define DFU_REBOOT_CMD_ID 0x58
#define DFU_REBOOT_CMD_NAME "DFU_REBOOT"
#define DFU_REBOOT_NUM_VALUES 0
#define DFU_GETVERSION_CMD_ID 0x59
#define DFU_GETVERSION_CMD_NAME "DFU_GETVERSION"
#define DFU_GETVERSION_NUM_VALUES 3

    /** Command resource ID */
//    control_resid_t res_id;
    /** Command name */
//    std::string cmd_name;
    /** Command value type */
//    cmd_param_type_t type;
    /** Command ID */
//    control_cmd_t cmd_id;
    /** Command read/write type */
//    cmd_rw_t rw;
    /** Number of values the command reads/writes */
 //   unsigned num_values;

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

control_ret_t getstatus(uint8_t &status, dfu_state &state)
{
    uint8_t values[DFU_GETSTATUS_NUM_VALUES];
    control_ret_t cmd_ret;

    cmd_ret = command_get(NULL, DFU_CONTROLLER_SERVICER_RESID, DFU_GETSTATUS_CMD_NAME, DFU_GETSTATUS_CMD_ID, DFU_GETSTATUS_NUM_VALUES, values);
    if (cmd_ret != CONTROL_SUCCESS) {
        cout << "Error: Command " << DFU_UPLOAD_CMD_NAME << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }

    uint32_t poll_timeout = ((0xff & values[3]) << 16) |
                            ((0xff & values[2]) << 8)  |
                            (0xff & values[1]);
    state = (dfu_state) values[4];
    status = values[5];
    printf("DFU_GETSTATUS: Status %d, State %d, Timeout (ms) %d\n", status, state, poll_timeout);

    std::this_thread::sleep_for(std::chrono::milliseconds(poll_timeout));

    return cmd_ret;
}

uint32_t status_is_idle() {
    uint8_t status;
    dfu_state state;
    if (getstatus(status, state) ==  CONTROL_SUCCESS)
    {
        if (state == DFU_STATE_appIDLE) {
            return 1;
        }
    }
    return 0;
}
control_ret_t setalternate(uint8_t alternate)
{
    uint8_t values[DFU_GETSTATUS_NUM_VALUES];
    control_ret_t cmd_ret;

    cmd_ret = command_set(NULL, DFU_CONTROLLER_SERVICER_RESID, DFU_SETALTERNATE_CMD_NAME, DFU_SETALTERNATE_CMD_ID, DFU_SETALTERNATE_NUM_VALUES, &alternate);
    if (cmd_ret != CONTROL_SUCCESS) {
        cout << "Error: Command " << DFU_SETALTERNATE_CMD_NAME << " returned error " << cmd_ret << endl;
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
    dfu_state state;
    size_t file_size = rf.tellg();
    uint32_t total_bytes = 0;
    control_ret_t cmd_ret;
    uint8_t is_state_not_idle = 1;

    while (total_bytes <= file_size) {
        uint8_t * values = new uint8_t[DFU_UPLOAD_NUM_VALUES];
        rf.read((char*) values, DFU_UPLOAD_NUM_VALUES);
        cmd_ret = command_set(NULL, DFU_CONTROLLER_SERVICER_RESID, DFU_DOWNLOAD_CMD_NAME, DFU_DOWNLOAD_CMD_ID, DFU_DOWNLOAD_NUM_VALUES, values);
        if (cmd_ret != CONTROL_SUCCESS) {
            cout << "Error: Command " << DFU_DOWNLOAD_CMD_NAME << " returned error " << cmd_ret << endl;
            return cmd_ret;
        }
        while (is_state_not_idle)
        {
            if (getstatus(status, state) ==  CONTROL_SUCCESS)
            {
                switch(state) {
                    case DFU_STATE_dfuDNLOAD_IDLE:
                        is_state_not_idle = 0;
                    break;
                    case DFU_STATE_dfuERROR:
                        // TODO: Handle error
                    break;
                    default:
                        continue;
                    break;
                }
            }
        }
        total_bytes += DFU_UPLOAD_NUM_VALUES;
    }
    rf.close();
    if(!rf.good()) {
        cout << "Error: Reading from file " << image_path << " failed" << endl;
        return CONTROL_ERROR;
    }
    // Send empty download message. TODO: Check how to indicate the zero payload length
    cmd_ret = command_set(NULL, DFU_CONTROLLER_SERVICER_RESID, DFU_DOWNLOAD_CMD_NAME, DFU_DOWNLOAD_CMD_ID, 0, NULL);
    if (cmd_ret != CONTROL_SUCCESS) {
        cout << "Error: Command " << DFU_DOWNLOAD_CMD_NAME << " returned error " << cmd_ret << endl;
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
                    cout << "Error: Invalid state: " << state << endl;
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
    control_ret_t cmd_ret = command_set(NULL, DFU_CONTROLLER_SERVICER_RESID, DFU_DETACH_CMD_NAME, DFU_DETACH_CMD_ID, DFU_DETACH_NUM_VALUES, NULL);
    if (cmd_ret != CONTROL_SUCCESS) {
        cout << "Error: Command " << DFU_SETALTERNATE_CMD_NAME << " returned error " << cmd_ret << endl;
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
        control_ret_t cmd_ret;
        uint8_t * values = new uint8_t[DFU_UPLOAD_NUM_VALUES];
        cmd_ret = command_get(NULL, DFU_CONTROLLER_SERVICER_RESID, DFU_UPLOAD_CMD_NAME, DFU_UPLOAD_CMD_ID, DFU_UPLOAD_NUM_VALUES, values);
        if (cmd_ret != CONTROL_SUCCESS) {
            cout << "Error: Command " << DFU_UPLOAD_CMD_NAME << " returned error " << cmd_ret << endl;
            return cmd_ret;
        }
        wf.write((const char *) values, DFU_UPLOAD_NUM_VALUES);
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

int main(int argc, char ** argv)
{
    if(argc == 1)
    {
        cout << "Use --help to get the list of options for this application." << endl
        << "Or use --list-commands to print the list of commands and their info." << endl;
        return 0;
    }

    string device_dl_name = get_device_lib_name(&argc, argv, options, num_options);
    int * device_init_info = NULL;
    if(device_dl_name == device_i2c_dl_name)
    {
        device_init_info = &i2c_info;
    } else if(device_dl_name == device_spi_dl_name)
    {
        device_init_info = &spi_info[0];
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
            setalternate(ALT_FACTORY_ID);
        } else {
            setalternate(ALT_UPGRADE_ID);
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
