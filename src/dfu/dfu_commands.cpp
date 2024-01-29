// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "dfu_commands.hpp"

using namespace std;

control_ret_t CommandList::command_get(Device * device, string cmd_name, uint8_t * values)
{
    int cmd_id = get_cmd_id(cmd_name) | 0x80; // setting 8th bit for read commands
    size_t num_values = get_cmd_length(cmd_name);
    size_t data_len = num_values + 1; // one extra for the status
    uint8_t * data = new uint8_t[data_len];

    control_ret_t ret = device->device_get(get_dfu_controller_servicer_resid(), cmd_id, data, data_len);
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
            ret = device->device_get(get_dfu_controller_servicer_resid(), cmd_id, data, data_len);
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
};

control_ret_t CommandList::command_set(Device * device, string cmd_name, uint8_t * values)
{
    int cmd_id = get_cmd_id(cmd_name); // setting 8th bit for read commands
    size_t num_values = get_cmd_length(cmd_name);
    size_t data_len = num_values;
    uint8_t * data = new uint8_t[data_len];

    for (unsigned i = 0; i < num_values; i++)
    {
        memcpy(data + i, &values[i], 1);
    }

    control_ret_t ret = device->device_set(get_dfu_controller_servicer_resid(), cmd_id, data, data_len);
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
            ret = device->device_set(get_dfu_controller_servicer_resid(), cmd_id, data, data_len);
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
};

void CommandList::add_command(YAML::Node yaml_info, const string cmd_name, uint8_t is_verbose)
{
    for (const auto& command : yaml_info)
    {
        if (command["cmd"].as<string>().find(cmd_name) != string::npos)
        {
            int cmd_id = command["index"].as<int>();
            int cmd_num_values = command["number_of_values"].as<int>();
            set_cmd_id(cmd_name, cmd_id);
            set_cmd_length(cmd_name, cmd_num_values);
            if (is_verbose) {
                cout << "Added command " << cmd_name << " with ID " << CommandIDs[cmd_name] << " and number of values " << cmd_num_values << endl;
            }
            return;
        }
    }
    cerr << "command " << cmd_name << " not found in YAML file" << endl;
    exit(HOST_APP_ERROR);
};

void CommandList::parse_dfu_cmds_yaml(string yaml_file_full_path, uint8_t is_verbose)
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
            if (is_verbose) {
                cout << "DFU_CONTROLLER_SERVICER_RESID is " << resource_id << endl;
            }
            set_dfu_controller_servicer_resid(resource_id);
        }
        YAML::Node dedicated_commands = node.second["dedicated_commands"];
        if (dedicated_commands.IsSequence())
        {
            for (const auto& cmd_name : commandNames)
            {
                add_command(dedicated_commands, cmd_name, is_verbose);
            }
        }
    }
    if (get_dfu_controller_servicer_resid() == -1)
    {
        cerr << "DFU_CONTROLLER_SERVICER_RESID not set. Check the YAML file: " << yaml_file_full_path << endl;
        exit(HOST_APP_ERROR);
    }
};