// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "dfu_operations.hpp"

using namespace std;

const std::string dfu_state_to_string( int state )
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

    return std::string(message);
}

const string dfu_status_to_string(int status)
{
    if (status > DFU_STATUS_errSTALLEDPKT)
        return "INVALID";
    return string(dfu_status_names[status]);
}

control_ret_t getstatus(Device * device, CommandList* command_list, uint8_t &status, uint8_t &state, uint8_t is_verbose)
{

    const string command_name = "DFU_GETSTATUS";
    uint8_t values[command_list->get_cmd_length(command_name)];
    if (is_verbose) {
        cout << "Send DFU_GETSTATUS message" << endl;
    }
    control_ret_t cmd_ret = command_list->command_get(device, command_name, values);
    if (cmd_ret != CONTROL_SUCCESS) {
        cerr << "Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }
    status = values[0];
    uint32_t poll_timeout = ((0xff & values[3]) << 16) |
                            ((0xff & values[2]) << 8)  |
                            (0xff & values[1]);
    state = values[4];
    if (is_verbose) {
        cout << "DFU_GETSTATUS: Status " << dfu_status_to_string(status) << ", State " << dfu_state_to_string(state) << ", Timeout (ms) " << poll_timeout << endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(poll_timeout));

    return cmd_ret;
}

control_ret_t clearStatus(Device * device, CommandList* command_list, uint8_t is_verbose)
{
    control_ret_t cmd_ret = CONTROL_SUCCESS;
    string command_name = "DFU_CLRSTATUS";
    uint8_t num_values = command_list->get_cmd_length(command_name);
    uint8_t values[num_values];
    if (is_verbose) {
        cout << "Send DFU_CLRSTATUS message" << endl;
    }
    cmd_ret = command_list->command_set(device, command_name, values);
    if (cmd_ret != CONTROL_SUCCESS) {
        cerr << "Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }

    return cmd_ret;
}

uint32_t status_is_idle(Device * device, CommandList* command_list, uint8_t is_verbose)
{
    uint8_t status;
    uint8_t state;

    if (getstatus(device, command_list, status, state, is_verbose) ==  CONTROL_SUCCESS)
    {
        switch(state) {
            case DFU_STATE_dfuERROR:
                clearStatus(device, command_list, is_verbose);
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

control_ret_t setalternate(Device * device, CommandList* command_list, uint8_t alternate, uint8_t is_verbose)
{
    control_ret_t cmd_ret = CONTROL_SUCCESS;
    string command_name = "DFU_SETALTERNATE";
    uint8_t num_values = command_list->get_cmd_length(command_name);
    uint8_t values[num_values];
    values[0] = alternate;
    if (is_verbose) {
        cout << "Send DFU_SETALTERNATE message with value " << unsigned(alternate) << endl;
    }
    cmd_ret = command_list->command_set(device, command_name, values);
    if (cmd_ret != CONTROL_SUCCESS) {
        cerr << "Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }

    return cmd_ret;
}

control_ret_t download_operation(Device * device, CommandList* command_list, const string image_path, uint8_t is_verbose)
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
    uint8_t num_values = command_list->get_cmd_length(command_name);
    uint8_t * values = new uint8_t[num_values];
    const uint32_t transfer_block_size = num_values - DFU_TRANSFER_BLOCK_LENGTH_BYTES;
    while (rf.good()) {

        for (int i=0; i<DFU_TRANSFER_BLOCK_LENGTH_BYTES; i++)
        {
            values[i] = (transfer_block_size>>8*i) & 0xFF;
        }
        rf.read((char*) &values[DFU_TRANSFER_BLOCK_LENGTH_BYTES], transfer_block_size);
        is_state_not_dn_idle = 1;
        if (is_verbose) {
            cout << "Send DFU_DNLOAD message with " << transfer_block_size << " bytes" << endl;
        }
        cmd_ret = command_list->command_set(device, command_name, values);
        if (cmd_ret != CONTROL_SUCCESS) {
            cerr << "Command " << command_name << " returned error " << cmd_ret << endl;
            return cmd_ret;
        }
        // Wait till device is in state dfuDNLOAD_IDLE
        while (is_state_not_dn_idle)
        {
            if (getstatus(device, command_list, status, state, is_verbose) ==  CONTROL_SUCCESS)
            {
                switch(state) {
                    case DFU_STATE_dfuDNLOAD_IDLE:
                        is_state_not_dn_idle = 0;
                    break;
                    case DFU_STATE_dfuERROR:
                        clearStatus(device, command_list, is_verbose);
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
        if (is_verbose) {
            cout << endl;
        }
        total_bytes += transfer_block_size;
    }
    cout << endl;


    rf.close();

    // Send empty download message
    cout << "Download completed. Send DFU_DNLOAD message with size zero" << endl;
    memset(values, 0, command_list->get_cmd_length(command_name));
    cmd_ret = command_list->command_set(device, command_name, values);
    if (cmd_ret != CONTROL_SUCCESS) {
        cerr << "Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }

    // Wait till device is in state dfuIDLE
    is_state_not_dn_idle = 1;
    while (!status_is_idle(device, command_list, is_verbose)) { }
    return CONTROL_SUCCESS;
}

control_ret_t reboot_operation(Device * device, CommandList* command_list, uint8_t is_verbose)
{
    cout << "Reboot device" << endl;
    control_ret_t cmd_ret = CONTROL_SUCCESS;

    string command_name = "DFU_DETACH";
    uint8_t num_values = command_list->get_cmd_length(command_name);
    uint8_t values[num_values];
    if (is_verbose) {
        cout << "Send DFU_DETACH message" << endl;
    }
    cmd_ret = command_list->command_set(device, command_name, values);
    if (cmd_ret != CONTROL_SUCCESS) {
        cerr << "Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }

    return cmd_ret;
}

control_ret_t upload_operation(Device * device, CommandList* command_list, const string image_path, uint8_t is_verbose)
{
    cout << "Uploading image to " << image_path << endl;
    ofstream wf(image_path, ios::out | ios::binary);

    string command_name = "DFU_UPLOAD";
    uint8_t num_values = command_list->get_cmd_length(command_name);
    uint8_t values[num_values];
    uint32_t transfer_block_num = 0;
    uint32_t transfer_ongoing = 1;
    uint32_t transfer_block_size = 0;
    const uint16_t dfu_data_buffer_size = num_values - DFU_TRANSFER_BLOCK_LENGTH_BYTES;

    if(!wf) {
        cout << "Cannot open file!" << endl;
        return CONTROL_ERROR;
    }
    while (transfer_ongoing) {
        transfer_block_size = 0;
        if (is_verbose) {
            cout << "Send DFU_UPLOAD message" << endl;
        }
        control_ret_t cmd_ret = command_list->command_get(device, command_name, values);
        if (cmd_ret != CONTROL_SUCCESS) {
            cerr << "Command " << command_name << " returned error " << cmd_ret << endl;
            return cmd_ret;
        }
        for (int i=0; i<DFU_TRANSFER_BLOCK_LENGTH_BYTES; i++)
        {
            transfer_block_size |= (values[i] << 8*i);
        }
        if (transfer_block_size) {
            cout << "\rUploaded " << transfer_block_num+1 << " blocks of " << transfer_block_size << " bytes" << std::flush;
            wf.write((const char *) &values[DFU_TRANSFER_BLOCK_LENGTH_BYTES], transfer_block_size);
        }
        if (is_verbose) {
            cout << endl;
        }
        // Wait till we receive a DFU_UPLOAD with no data
        if (transfer_block_size < dfu_data_buffer_size) {
            cout << "\nReceived transport block with size " << transfer_block_size << " (smaller than "<< dfu_data_buffer_size << "): upload complete" << endl;
            transfer_ongoing = 0;
        }
        transfer_block_num++;

    }

    wf.close();
    if(!wf.good()) {
        cerr << "Writing to file " << image_path << " failed" << endl;
        return CONTROL_ERROR;
    }
    return CONTROL_SUCCESS;
}

control_ret_t getversion(Device * device, CommandList* command_list, uint8_t is_verbose)
{

    const string command_name = "DFU_GETVERSION";
    uint8_t values[command_list->get_cmd_length(command_name)];
    if (is_verbose) {
        cout << "Send DFU_GETVERSION message" << endl;
    }
    control_ret_t cmd_ret = command_list->command_get(device, command_name, values);
    if (cmd_ret != CONTROL_SUCCESS) {
        cerr << "Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }
    cout << "DFU_GETVERSION: ";
    for (int i=0; i<command_list->get_cmd_length(command_name); i++) {
        cout << unsigned(values[i]) << " ";
    }
    cout << endl;

    return cmd_ret;
}

control_ret_t set_transport_block(Device * device, CommandList* command_list, uint16_t block_number, uint8_t is_verbose)
{
    cout << "Set transport block number " << block_number << endl;
    control_ret_t cmd_ret = CONTROL_SUCCESS;

    string command_name = "DFU_TRANSFERBLOCK";
    uint8_t num_values = command_list->get_cmd_length(command_name);
    uint8_t values[num_values];
    values[0] = block_number&0xFF;
    values[1] = block_number>>8;

    if (is_verbose) {
        cout << "Send DFU_TRANSFERBLOCK message" << endl;
    }
    cmd_ret = command_list->command_set(device, command_name, values);
    if (cmd_ret != CONTROL_SUCCESS) {
        cerr << "Command " << command_name << " returned error " << cmd_ret << endl;
        return cmd_ret;
    }

    return cmd_ret;
}