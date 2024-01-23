// Copyright 2022-2024 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "special_commands.hpp"
#include <fstream>
#include <iomanip>
#include <ctype.h>

#if (defined(__APPLE__) || defined(_WIN32))
#include <sstream>
#endif

using namespace std;

extern size_t num_commands;

string get_cmd_map_abs_path(int * argc, char ** argv)
{
    string cmd_map_rel_path = default_command_map_name;
    string cmd_map_abs_path = "";
    opt_t * cmp_opt = option_lookup("--command-map-path", options, num_options);
    size_t index = argv_option_lookup(*argc, argv, cmp_opt);
    if(index != 0)
    {
        // Use path given via CLI
        cmd_map_rel_path = argv[index + 1];
        remove_opt(argc, argv, index, 2);
        cmd_map_abs_path = convert_to_abs_path(cmd_map_rel_path);
    }
    else
    {
        cmd_map_abs_path = get_dynamic_lib_path(cmd_map_rel_path);
    }
    return cmd_map_abs_path;
}

bool get_bypass_range_check(int * argc, char ** argv)
{
    opt_t * bp_opt = option_lookup("--bypass-range-check", options, num_options);
    size_t index = argv_option_lookup(*argc, argv, bp_opt);
    if(index == 0)
    {
        // if option is not present bypass is false
        return false;
    }
    else
    {
        remove_opt(argc, argv, index, 1);
        return true;
    }
}

uint8_t get_band_option(int * argc, char ** argv)
{
    opt_t *band_opt = option_lookup("--band", options, num_options);
    size_t index = argv_option_lookup(*argc, argv, band_opt);
    if (index == 0) // --band not provided. Default to band 0
    {
        return 0;
    }
    else
    {
        if (isdigit(argv[index+1][0])) // Get the actual band index that follows the --band option
        {
            int band = atoi(argv[index+1]);
            if((band != 0) && (band != 1))
            {
                cerr << "Invalid band index provided after the --band option. Provide either 0 or 1" << endl;
                exit(HOST_APP_ERROR);
            }
            remove_opt(argc, argv, index, 2);
            return band;
        }
        else
        {
            cerr << "No band index provided after the --band option. Provide either 0 or 1" << endl;
            exit(HOST_APP_ERROR);
        }
    }
}

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
    << endl << "You can use --use or -u option to specify protocol you want to use"
    << endl << "or call the option/command directly using default control protocol."
    << endl << "Default control protocol is USB."
    << endl << "You can use --bypass-range-check or -br to bypass parameter range checking."
    << endl << "Range check is True unless -br is specified."
    << endl << "You can use --command-map-path or -cmp to specify the comand_map object to use."
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

control_ret_t print_command_list()
{
    size_t longest_command = 0;
    size_t longest_rw = 10; // READ/WRITE
    size_t longest_args = 2; // double digits
    size_t longest_type = 7; // radians
    for(size_t i = 0; i < num_commands; i ++)
    {
        cmd_t cmd = {0};
        init_cmd(&cmd, "_", i);
        // skipping hidden commands
        if(cmd.hidden_cmd)
        {
            continue;
        }
        size_t name_len = cmd.cmd_name.length();
        longest_command = (longest_command < name_len) ? name_len : longest_command;
    }
    size_t rw_offset = longest_command + 2;
    size_t args_offset = rw_offset + longest_rw + 2;
    size_t type_offset = args_offset + longest_args + 2;
    size_t info_offset = type_offset + longest_type + 2;
    // Getting current terminal width here to set the cout line limit
    const size_t hard_stop = get_term_width();

    for(size_t i = 0; i < num_commands; i ++)
    {
        cmd_t cmd = {0};
        init_cmd(&cmd, "_", i);
        // skipping hidden commands
        if(cmd.hidden_cmd)
        {
            continue;
        }
        // name   rw   args   type   info
        size_t name_len = cmd.cmd_name.length();
        string rw = command_rw_type_name(cmd.rw);
        size_t rw_len = rw.length();
        size_t args_len = to_string(cmd.num_values).length();
        string type = command_param_type_name(cmd.type);
        size_t type_len = type.length();
        size_t first_word_len = cmd.info.find_first_of(' ');

        int first_space = rw_offset - name_len + rw_len;
        int second_space = args_offset - rw_len - rw_offset + args_len;
        int third_space = type_offset - args_len - args_offset + type_len;
        int fourth_space = info_offset - type_len - type_offset + first_word_len;

        cout << cmd.cmd_name << setw(first_space) << rw
        << setw(second_space) << cmd.num_values << setw(third_space)
        << type << setw(fourth_space);

        stringstream ss(cmd.info);
        string word;
        size_t curr_pos = info_offset;
        while(ss >> word)
        {
            size_t word_len = word.length();
            size_t future_pos = curr_pos + word_len + 1;
            if(future_pos >= hard_stop)
            {
                cout << endl << setw(info_offset + word_len) << word << " ";
                curr_pos = info_offset + word_len;
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

control_ret_t dump_params(Command * command)
{
    for(size_t i = 0; i < num_commands; i ++)
    {
        cmd_t cmd = {0};
        init_cmd(&cmd, "_", i);
        // skipping hidden commands
        if(cmd.hidden_cmd)
        {
            continue;
        }
        if(cmd.rw != CMD_WO)
        {
            command->do_command(cmd.cmd_name, nullptr, 0, 0);
        }
    }
    return CONTROL_SUCCESS;
}

control_ret_t execute_cmd_list(Command * command, const string filename)
{
    size_t largest_command = 0;
    for(size_t i = 0; i < num_commands; i++)
    {
        cmd_t cmd = {0};
        init_cmd(&cmd, "_", i);
        size_t num_args = cmd.num_values;
        largest_command = (num_args > largest_command) ? num_args : largest_command;
    }
    largest_command++; // +1 for the command name
    ifstream file(filename, ios::in);
    if(!file)
    {
        cerr << "Could not open a file " << filename << endl;
        exit(HOST_APP_ERROR);
    }
    string line;
    while(getline(file, line))
    {
        // TODO: think about -e use cases whether 128 bytes per line is enough
        const size_t max_line_len = 128;
        char buff[max_line_len];
        int i = 0;

        char ** line_ch = new char * [largest_command];
        int num = 0;
        stringstream ss(line);
        string word;
        // if newline
        if(ss.peek() == -1)
        {
            continue;
        }
        while(ss >> word)
        {
            memcpy(&buff[i], word.c_str(), word.length());
            buff[i + word.length()] = '\0';
            line_ch[num] = &buff[i];
            i += word.length() + 2;
            if(i > max_line_len)
            {
                cerr << "Line:" << endl << line
                << endl << "Exceeded " << max_line_len
                << " characters limit" << endl;
                return CONTROL_BAD_COMMAND;
            }
            num++;
        }
        int cmd_indx = 0;
        int arg_indx =  cmd_indx + 1; // 0 is the command
        command->do_command(line_ch[cmd_indx], line_ch, num, arg_indx);
        delete []line_ch;
    }
    file.close();
    return CONTROL_SUCCESS;
}

control_ret_t test_bytestream(Command * command, const string in_filename)
{
    control_ret_t ret;
    ifstream rf(in_filename, ios::in | ios::binary);
    if(!rf)
    {
        cerr << "Could not open a file " << in_filename << endl;
        exit(HOST_APP_ERROR);
    }
    rf.seekg (0, rf.end);
    size_t size = (size_t) rf.tellg();
    rf.seekg (0, rf.beg);

    uint8_t *data = new uint8_t[size];
    for(unsigned int i=0; i<size; i++)
    {
        rf.read(reinterpret_cast<char *>(&data[i]), sizeof(uint8_t));
    }

    if((size >= 2) && (data[1] & 0x80)) // Read command
    {
        ret = command->command_get_low_level(data, size);
    }
    else
    {
        ret = command->command_set_low_level(data, size);
    }

    delete []data;

    return ret;
}

control_ret_t test_control_interface(Command * command, const string out_filename)
{
    control_ret_t  ret = CONTROL_ERROR;
    int test_frames = 50;

    const string test_cmd_name = "TEST_CONTROL";
    cmd_t test_cmd = {0};
    init_cmd(&test_cmd, test_cmd_name);
    cmd_param_t * test_in_buffer = new cmd_param_t[test_cmd.num_values * test_frames];
    cmd_param_t * test_out_buffer = new cmd_param_t[test_cmd.num_values * test_frames];
    size_t num_all_vals = test_frames * test_cmd.num_values;

    string in_filename = "test_input_buf.bin";
    ifstream rf(in_filename, ios::out | ios::binary);
    if(!rf)
    {
        cerr << "Could not open a file " << in_filename << endl;
        exit(HOST_APP_ERROR);
    }

    rf.seekg (0, rf.end);
    streamoff size = rf.tellg();
    rf.seekg (0, rf.beg);

    if(size != (num_all_vals * sizeof(uint8_t)))
    {
        cerr << "Test buffer lengths don't match" << endl;
        exit(HOST_APP_ERROR);
    }

    for(size_t i = 0; i < num_all_vals; i++)
    {
        rf.read(reinterpret_cast<char *>(&test_in_buffer[i].ui8), sizeof(uint8_t));
    }

    rf.peek(); // doing peek here to look for a character beyond file size so it will set eof
    rf.close();

    if(!rf.eof() || rf.bad())
    {
        cerr << "Error occured while reading " << in_filename << endl;
        exit(HOST_APP_ERROR);
    }
    command->init_cmd_info(test_cmd_name);
    for(int n = 0; n < test_frames; n++)
    {
        ret = command->command_set(&test_in_buffer[n * test_cmd.num_values]);

        ret = command->command_get(&test_out_buffer[n * test_cmd.num_values]);
    }
    delete []test_in_buffer;

    cout << "filename is " << out_filename << endl;
    ofstream wf(out_filename, ios::out | ios::binary);
    if(!wf)
    {
        cerr << "Could not open a file " << out_filename << endl;
        exit(HOST_APP_ERROR);
    }

    for(size_t i = 0; i < num_all_vals; i++)
    {
        wf.write(reinterpret_cast<char *>(&test_out_buffer[i].ui8), sizeof(uint8_t));
    }
    delete []test_out_buffer;

    wf.close();
    if(wf.bad())
    {
        cerr << "Error occured when writing to " << out_filename << endl;
        exit(HOST_APP_ERROR);
    }
    return ret;
}
