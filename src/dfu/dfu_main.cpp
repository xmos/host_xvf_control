// Copyright 2024 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "dfu_operations.hpp"
#include <sys/stat.h> // stat

using namespace std;

/** @brief List of supported CLI options */
opt_t options[] = {
    {"--help",                    "-h",        "display this information"                                                                                           },
    {"--app-version",             "-av",       "print the version of this application",                                                                             },
    {"--use",                     "-u",        "use specific hardware protocol, I2C, SPI and USB are available to use"                                              },
    {"--verbose",                 "-vvv",      "enable debug prints"                                                                                                },
    {"--upload-start",            "-us",       "set the first block transport number for the upload operation. Default is 0. Option valid only with upload commands"},
    {"--version",                 "-v",        "read the version on the device",                                                                                    },
    {"--download",                "-d",        "download upgrade image stored in the specified path"                                                                },
    {"--upload-factory",          "-uf",       "upload factory image and save it in the specified path"                                                             },
    {"--upload-upgrade",          "-uu",       "upload upgrade image and save it in the specified path"                                                             },
    {"--reboot",                  "-r",        "reboot device"                                                                                                      },
};
size_t num_options = end(options) - begin(options);

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
    cout << "usage: xvf_dfu [ -u <protocol> ] command" << endl
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

uint8_t check_verbose(int * argc, char ** argv)
{
    opt_t * opt = option_lookup("--verbose", options, num_options);
    size_t index = argv_option_lookup(*argc, argv, opt);
    if(index != 0)
    {
        cout << "Verbose mode enabled" << endl;
        remove_opt(argc, argv, index, 1);
        return 1;
    }
    return 0;
}

uint16_t check_upload_start(int * argc, char ** argv)
{
    opt_t * opt = option_lookup("--upload-start", options, num_options);
    size_t index = argv_option_lookup(*argc, argv, opt);
    // return -1 if the upload-start option is not found
    if (index == 0) {
        return INVALID_TRANSPORT_BLOCK_NUM;
    }
    if (index + 1 >= *argc)
    {
        cerr << "Missing block number" << endl;
        exit(HOST_APP_ERROR);
    }
    string block_number_str = argv[index + 1];
    if (!isdigit(block_number_str[0])) {
        cerr << "Value for transport block is not an integer. Given value: " << block_number_str << endl;
        exit(HOST_APP_ERROR);
    }
    if (stoi(block_number_str) >= INVALID_TRANSPORT_BLOCK_NUM)
    {
        cerr << "Max value for transport block is " << INVALID_TRANSPORT_BLOCK_NUM-1 << ". Given value: " << block_number_str << endl;
        exit(HOST_APP_ERROR);
    }
    remove_opt(argc, argv, index, 2);

    return stoi(block_number_str);
}
int main(int argc, char ** argv)
{
    if(argc == 1)
    {
        cout << "Use --help to get the list of options for this application." << endl;
        return 0;
    }
    YAML::Node config;
    int* i2c_info = new int[1];
    int* spi_info = new int[2];
    string yaml_file_name;

    // Check if --use option is used
    string device_dl_name = get_device_lib_name(&argc, argv, options, num_options);

    // Check other optional arguments
    uint8_t is_verbose = check_verbose(&argc, argv);
    uint16_t start_block_number = check_upload_start(&argc, argv);

    // Load YAML file with transport settings
    yaml_file_name = get_executable_path() + "/transport_config.yaml";
    if (is_verbose) {
        cout << "Parsing YAML file " << yaml_file_name << endl;
    }
    if (is_file_found(yaml_file_name)) {
        config = YAML::LoadFile(yaml_file_name);
        // Read I2C parameters from YAML file
        i2c_info[0] = config["I2C_ADDRESS"].as<int>();
        spi_info[0] = config["SPI_MODE"].as<int>();
        spi_info[1] = 1024;
    } else {
        cerr << "File \'" << yaml_file_name << "\' not found" << endl;
        exit(HOST_APP_ERROR);
    }

    // Check first CLI commands which don't require access to the device
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
    CommandList* command_list = new CommandList();
    // Load YAML file with DFU commands info
    yaml_file_name = get_executable_path() + "/dfu_cmds.yaml";
    if (is_verbose) {
        cout << "Parsing YAML file " << yaml_file_name << endl;
    }
    if (is_file_found(yaml_file_name)) {
        command_list->parse_dfu_cmds_yaml(yaml_file_name, is_verbose);
    } else {
        cerr << "File \'" << yaml_file_name << "\' not found" << endl;
        exit(HOST_APP_ERROR);
    }

    // Check CLI options which require access to the device
    next_cmd = argv[cmd_indx];
    if(next_cmd[0] == '-')
    {
        opt = option_lookup(next_cmd, options, num_options);
        int arg_indx = cmd_indx + 1;

        // Check if upload-start is used in combination with upload commands
        if (start_block_number != INVALID_TRANSPORT_BLOCK_NUM) {
            if (opt->long_name != "--upload-factory" && opt->long_name != "--upload-upgrade")
            {
                cerr << "Option--upload-start is valid only with upload commands" << endl;
                exit(HOST_APP_ERROR);
            }
        }

        if (opt->long_name == "--version")
        {
            get_version(device, command_list, is_verbose);
            exit(0);
        }

        // Set appropriate ALT-SETTING
        if (opt->long_name == "--upload-factory")
        {
            set_alternate(device, command_list, DFU_ALT_FACTORY_ID, is_verbose);
        } else {
            set_alternate(device, command_list, DFU_ALT_UPGRADE_ID, is_verbose);
        }
        if (!state_is_idle(device, command_list, is_verbose)) {
            return -1;
        }

        if (opt->long_name == "--download")
        {
            string image_path = "";
            if (arg_indx >= argc)
            {
                cerr << "Missing file path" << endl;
                exit(HOST_APP_ERROR);
            } else {
                image_path = argv[arg_indx];
            }
            if (is_file_found(image_path)) {
                download_operation(device, command_list, image_path, is_verbose);
            } else {
                cerr << "File at path \'" << argv[arg_indx] << "\' not found" << endl;
                return -1;
            }
        }
        if (opt->long_name == "--reboot")
        {
            reboot_operation(device, command_list, is_verbose);
        }
        if (opt->long_name == "--upload-factory" || opt->long_name == "--upload-upgrade")
        {
            string image_path = "";
            if (arg_indx >= argc)
            {
                cerr << "Missing file path" << endl;
                exit(HOST_APP_ERROR);
            } else {
                image_path = argv[arg_indx];
            }
            if (start_block_number!=INVALID_TRANSPORT_BLOCK_NUM) {
                set_transport_block(device, command_list, start_block_number, is_verbose);
            }
            if (!is_file_found(image_path)) {
                upload_operation(device, command_list, image_path, is_verbose);
            } else {
                cerr << "File at path \'" << argv[arg_indx] << "\' already exists" << endl;
                return -1;
            }
        }
    }
}
