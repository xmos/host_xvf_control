// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "special_commands.hpp"
#include "dlfcn.h"
#include <fstream>
#include <iomanip>

using namespace std;

opt_t options[] = {
            {"--help",                    "-h",        "display this information",            "0"                                        },
            {"--list-commands",           "-l",        "print the list of commands",          "0"                                        },
            {"--dump-params",             "-d",        "print all the parameters",            "0"                                        },
            {"--execute-command-list",    "-e",        "execute commands from .txt file,",    "one command per line, don't need -u *"    },
            {"--use",                     "-u",        "use specific harware protocol,",      "I2C and SPI are available to use"         },
            {"--get-aec-filter",          "-gf",       "get AEC filter into .bin files,",     "default is aec_filter.bin.fx.mx"          },
            {"--set-aec-filter",          "-sf",       "set AEC filter from .bin files,",     "default is aec_filter.bin.fx.mx"          },
            {"--get-nlmodel-buffer",      "-gn",       "get NLModel filter into .bin file,",  "default is nlm_buffer.bin"                },
            {"--set-nlmodel-buffer",      "-sn",       "set NLModel filter from .bin file,",  "default is nlm_buffer.bin"                },
            {"--test-control-interface",  "-tc",       "test control interface",              "default is test_buffer.bin"               }
};
size_t num_options = end(options) - begin(options);

cmd_t * commands;
size_t num_commands;

void * load_command_map_dll()
{
    string dyn_lib_path = get_dynamic_lib_path("libcommand_map");
    void * handle = dlopen(dyn_lib_path.c_str(), RTLD_NOW);
    if(handle == NULL)
    {
        cerr << "Error while opening " << dyn_lib_path << endl;
        exit(CONTROL_ERROR);
    }
    // declaring cmd_t * function pointer type
    using cmd_map_t = cmd_t * (*)();
    cmd_map_t get_command_map = reinterpret_cast<cmd_map_t>(dlsym(handle, "get_command_map"));
    if(get_command_map == NULL)
    {
        cerr << "Error while loading get_command_map() from libcommand_map" << endl;
        exit(CONTROL_ERROR);
    }
    // declaring uint32_t function pointer type
    using num_cmd_t = uint32_t (*)();
    num_cmd_t get_num_commands = reinterpret_cast<num_cmd_t>(dlsym(handle, "get_num_commands"));
    if(get_num_commands == NULL)
    {
        cerr << "Error while loading get_num_commands() from libcommand_map" << endl;
        exit(CONTROL_ERROR);
    }

    commands = get_command_map();
    num_commands = get_num_commands();
    return handle;
}

opt_t * option_lookup(const string str)
{
    string low_str = to_lower(str);
    for(int i = 0; i < num_options; i++)
    {
        opt_t * opt = &options[i];
        if ((low_str == opt->long_name) || (low_str == opt->short_name))
        {
            return opt;
        }
    }

    int shortest_dist = 100;
    int indx  = 0;
    for(int i = 0; i < num_options; i++)
    {
        opt_t * opt = &options[i];
        int dist_long = levDistance(low_str, opt->long_name);
        int dist_short = levDistance(low_str, opt->short_name);
        int dist = (dist_short < dist_long) ? dist_short : dist_long;
        if(dist < shortest_dist)
        {
            shortest_dist = dist;
            indx = i;
        }
    }
    cerr << "Option " << str << " does not exist." << endl
    << "Maybe you meant " << options[indx].short_name
    << " or " << options[indx].long_name << "." << endl;
    exit(CONTROL_BAD_COMMAND);
    return nullptr;
}

cmd_t * command_lookup(const string str)
{
    string up_str = to_upper(str);
    for(size_t i = 0; i < num_commands; i++)
    {
        cmd_t * cmd = &commands[i];
        if (up_str == cmd->cmd_name)
        {
            return cmd;
        }
    }

    int shortest_dist = 100;
    int indx  = 0;
    for(int i = 0; i < num_commands; i++)
    {
        cmd_t * cmd = &commands[i];
        int dist = levDistance(up_str, cmd->cmd_name);
        if(dist < shortest_dist)
        {
            shortest_dist = dist;
            indx = i;
        }
    }
    cerr << "Command " << str << " does not exist." << endl
    << "Maybe you meant " << commands[indx].cmd_name <<  "." << endl;
    exit(CONTROL_BAD_COMMAND);
    return nullptr;
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

    cout << "usage: xvf_hostapp_rpi [ command | option ]" << endl
    << setw(68) << "[ -u | --use <protocol>] [ command | option ]" << endl
    << endl << "You can use --use option to specify protocol you want to use"
    << endl << "or call the option/command directly using default control protocol."
    << endl << "Default control protocol is I2C." << endl << endl << "Options:" << endl;
    for(opt_t opt : options)
    {
        size_t short_len = opt.short_name.length();
        size_t long_len = opt.long_name.length();
        size_t info_len = opt.info.length();
        int first_space = long_opt_offset - short_len + long_len;
        int second_space = info_offset - long_len - long_opt_offset + info_len;

        cout << "  " << opt.short_name <<  "," << setw(first_space) << opt.long_name
        << setw(second_space) << opt.info << endl;
        if(opt.more_info != "0")
        {
            size_t more_info_len = opt.more_info.length();
            int space = info_offset + more_info_len + 3; // +3 since we used two spaces and a comma in the line before
            cout << setw(space) << opt.more_info << endl;
        }
    }
    return CONTROL_SUCCESS;
}

control_ret_t print_command_list()
{
    string spec_cmd = "SPECIAL_CMD_";
    string test_cmd = "TEST_";
    size_t longest_command = 0;
    size_t longest_rw = 10; // READ/WRITE
    size_t longest_args = 2; // double digits
    size_t longest_type = 7; // radians
    size_t longest_info = 0;
    for(size_t i = 0; i < num_commands; i ++)
    {
        cmd_t * cmd = &commands[i];
        // skipping special and test commands
        if((cmd->cmd_name.find(spec_cmd) != string::npos) || (cmd->cmd_name.find(test_cmd) != string::npos))
        {
            continue;
        }
        size_t name_len = cmd->cmd_name.length();
        size_t info_len = cmd->info.length();
        longest_command = (longest_command < name_len) ? name_len : longest_command;
        longest_info = (longest_info < info_len) ? info_len : longest_info;
    }
    size_t rw_offset = longest_command + 2;
    size_t args_offset = rw_offset + longest_rw + 2;
    size_t type_offset = args_offset + longest_args + 2;
    size_t info_offset = type_offset + longest_type + 2;

    for(size_t i = 0; i < num_commands; i ++)
    {
        cmd_t * cmd = &commands[i];
        // skipping special and test commands
        if((cmd->cmd_name.find(spec_cmd) != string::npos) || (cmd->cmd_name.find(test_cmd) != string::npos))
        {
            continue;
        }
        // name   rw   args   type   info
        size_t name_len = cmd->cmd_name.length();
        string rw = command_rw_type_name(cmd->rw);
        size_t rw_len = rw.length();
        size_t args_len = to_string(cmd->num_values).length();
        string type = command_param_type_name(cmd->type);
        size_t type_len = type.length();
        size_t info_len = cmd->info.length();
        int first_space = rw_offset - name_len + rw_len;
        int second_space = args_offset - rw_len - rw_offset + args_len;
        int third_space = type_offset - args_len - args_offset + type_len;
        int fourth_space = info_offset - type_len - type_offset + info_len;
        cout << cmd->cmd_name << setw(first_space) << rw
        << setw(second_space) << cmd->num_values << setw(third_space)
        << type << setw(fourth_space) << cmd->info << endl;
    }
    return CONTROL_SUCCESS;
}

control_ret_t dump_params(Command * command)
{
    control_ret_t ret = CONTROL_ERROR;
    string spec_cmd = "SPECIAL_CMD_";
    string test_cmd = "TEST_";
    for(size_t i = 0; i < num_commands; i ++)
    {
        cmd_t * cmd = &commands[i];
        // skipping special and test commands
        if((cmd->cmd_name.find(spec_cmd) != string::npos) || (cmd->cmd_name.find(test_cmd) != string::npos))
        {
            continue;
        }
        if(cmd->rw != CMD_WO)
        {
            ret = command->do_command(&commands[i], nullptr, 0, 0);
        }
    }
    return ret;
}

control_ret_t execute_cmd_list(Command * command, const char * filename)
{
    control_ret_t ret = CONTROL_ERROR;
    size_t largest_command = 0;
    for(size_t i = 0; i < num_commands; i++)
    {
        cmd_t * cmd = &commands[i];
        size_t num_args = cmd->num_values;
        largest_command = (num_args > largest_command) ? num_args : largest_command;
    }
    largest_command++; // +1 for the command name
    ifstream file(filename);
    string line;
    while(getline(file, line))
    {
        //TODO: think about -e use cases whether 128 bytes per line is enough
        size_t max_line_len = 128;
        char buff[max_line_len];
        int i = 0;
        char * line_ch[largest_command];
        int num = 0;
        stringstream ss(line);
        string word;
        while(ss >> word)
        {
            strcpy(&buff[i], word.c_str());
            line_ch[num] = &buff[i];
            i += word.length() + 1;
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
        int arg_indx = cmd_indx + 1;
        int args_left = num - 1;
        cmd_t * cmd = command_lookup(line_ch[cmd_indx]);
        ret = command->do_command(cmd, line_ch, args_left, arg_indx);
    }
    file.close();
    return ret;
}

control_ret_t get_or_set_full_buffer(Command * command, cmd_param_t *buffer, int32_t buffer_length, cmd_t *start_coeff_index_cmd, cmd_t *filter_cmd, bool flag_buffer_get)
{
    control_ret_t ret;
    int32_t num_filter_read_commands = (buffer_length + filter_cmd->num_values - 1) / filter_cmd->num_values;

    int32_t start_coeff = 0;
    for(int i = 0; i < num_filter_read_commands; i++)
    {
        cmd_param_t coeff;
        coeff.i32 = start_coeff;
        ret = command->command_set(start_coeff_index_cmd, &coeff, 1);

        if(flag_buffer_get == true) // Read from the device into the buffer
        {
            ret = command->command_get(filter_cmd, &buffer[start_coeff], filter_cmd->num_values);
        }
        else // Write buffer to the device
        {
            ret = command->command_set(filter_cmd, &buffer[start_coeff], filter_cmd->num_values);
        }

        start_coeff += filter_cmd->num_values;
    }
    return ret;
}

control_ret_t get_one_filter(Command * command, int32_t mic_index, int32_t far_index, string filename, uint32_t buffer_length, bool flag_buffer_get)
{
    clog << "Filename = " << filename << endl;

    cmd_t * far_mic_index_cmd = command_lookup("SPECIAL_CMD_AEC_FAR_MIC_INDEX"); // Start cmd
    
    cmd_t * start_coeff_index_cmd = command_lookup("SPECIAL_CMD_AEC_FILTER_COEFF_START_OFFSET"); // Set start offset

    cmd_t * filter_cmd = command_lookup("SPECIAL_CMD_AEC_FILTER_COEFFS"); // Get buffer cmd

    // Set start of special command sequence
    cmd_param_t far_mic_index[2];
    far_mic_index[0].i32 = far_index;
    far_mic_index[1].i32 = mic_index;
    control_ret_t ret = command->command_set(far_mic_index_cmd, far_mic_index, far_mic_index_cmd->num_values);
    
    int32_t len = ((buffer_length + (filter_cmd->num_values - 1)) / filter_cmd->num_values) * filter_cmd->num_values;
    cmd_param_t * aec_filter = new cmd_param_t[len];

    if(flag_buffer_get == true)
    {
        // Read the full buffer from the device
        ret = get_or_set_full_buffer(command, aec_filter, buffer_length, start_coeff_index_cmd, filter_cmd, flag_buffer_get);

        // Write filter to file
        ofstream wf(filename, ios::out | ios::binary);
        if(!wf)
        {
            cerr << "Could not open a file " << filename << endl;
            exit(CONTROL_ERROR);
        }

        for(int i = 0; i < buffer_length; i++)
        {
            wf.write(reinterpret_cast<char *>(&aec_filter[i].f), sizeof(float));
        }

        wf.close();
        if(wf.bad())
        {
            cerr << "Error occured when writting to " << filename << endl;
            exit(CONTROL_ERROR);
        }
    }
    else
    {
        ifstream rf(filename, ios::out | ios::binary);
        if(!rf)
        {
            cerr << "Could not open a file " << filename << endl;
            exit(CONTROL_ERROR);
        }

        rf.seekg (0, rf.end);
        int32_t size = rf.tellg();
        rf.seekg (0, rf.beg);

        if(size != (buffer_length * sizeof(float)))
        {
            cerr << "AEC buffer lengths don't match" << endl;
            exit(CONTROL_DATA_LENGTH_ERROR);
        }

        // Read from file into aec buffer. Will need to be done byte by byte since aec_filter is of type cmd_param_t
        for(int i = 0; i < buffer_length; i++)
        {
            rf.read(reinterpret_cast<char *>(&aec_filter[i].f), sizeof(float));
        }

        rf.peek(); // doing peek here to look for a character beyond file size so it will set eof
        rf.close();

        if(!rf.eof() || rf.bad())
        {
            cerr << "Error occured while reading " << filename << endl;
            exit(CONTROL_ERROR);
        }

        ret = get_or_set_full_buffer(command, aec_filter, buffer_length, start_coeff_index_cmd, filter_cmd, flag_buffer_get);
    }
    delete []aec_filter;
    return ret;
}

control_ret_t special_cmd_aec_filter(Command * command, bool flag_buffer_get, const char *filename)
{
    cmd_t * num_mics_cmd = command_lookup("AEC_NUM_MICS");

    cmd_t * num_farends_cmd = command_lookup("AEC_NUM_FARENDS");

    cmd_param_t num_mics, num_farends;

    control_ret_t ret = command->command_get(num_mics_cmd, &num_mics, num_mics_cmd->num_values);

    ret = command->command_get(num_farends_cmd, &num_farends, num_farends_cmd->num_values);

     // Get AEC filter length
    cmd_t * aec_filter_length_cmd = command_lookup("SPECIAL_CMD_AEC_FILTER_LENGTH");

    cmd_param_t filt;
    ret = command->command_get(aec_filter_length_cmd, &filt, aec_filter_length_cmd->num_values);

    uint32_t filter_length = filt.i32;
    clog << "AEC filter length = " << filter_length << endl;

    // Set AEC to bypass to stop filter from adapting
    cmd_t * aec_bypass_cmd = command_lookup("AEC_BYPASS");

    cmd_param_t bypass;
    bypass.ui8 = 1;
    command->command_set(aec_bypass_cmd, &bypass, 1);


    // TODO attempt to run for only one (mic, farend) pair due to timing violation this
    // command is causing in AEC. Will re-enable getting all filters once we have the chunkwise API.
    for(int far_index = 0; far_index < num_farends.i32; far_index++)
    {
        for(int mic_index = 0; mic_index < num_mics.i32; mic_index++)
        {
            // Get AEC filter for the (far_index, mic_index) pair
            string filter_name = filename;
            filter_name += ".f" + to_string(far_index) + ".m" + to_string(mic_index) ;
            ret = get_one_filter(command, mic_index, far_index, filter_name, filter_length, flag_buffer_get);
        }
    }

    // Set AEC bypass to 0 to allow filter to adapt again
    bypass.ui8 = 0;
    command->command_set(aec_bypass_cmd, &bypass, 1);

    return ret;
}

control_ret_t special_cmd_nlmodel_buffer(Command * command, bool flag_buffer_get, const char* filename)
{
    cmd_t * nlm_buffer_start_cmd = command_lookup("SPECIAL_CMD_NLMODEL_START"); // Start cmd
    
    cmd_t * nlm_buffer_length_cmd = command_lookup("SPECIAL_CMD_PP_NLMODEL_NROW_NCOL"); // Get buffer length

    cmd_t * start_coeff_index_cmd = command_lookup("SPECIAL_CMD_NLMODEL_COEFF_START_OFFSET"); // Set start offset

    cmd_t * filter_cmd = command_lookup("SPECIAL_CMD_PP_NLMODEL"); // buffer cmd

    // Get buffer length
    int32_t NLM_buffer_length;
    cmd_param_t nRowCol[2];
    control_ret_t ret = command->command_get(nlm_buffer_length_cmd, nRowCol, nlm_buffer_length_cmd->num_values);

    string filter_name = filename;
    filter_name += ".r" + to_string(nRowCol[0].i32) + ".c" + to_string(nRowCol[1].i32);
    clog << "Filename = " << filter_name << endl;

    NLM_buffer_length = nRowCol[0].i32 * nRowCol[1].i32;
    clog << "NLM_buffer_length = " << NLM_buffer_length << endl;

    // Set start of special command sequence
    cmd_param_t start_buffer_read;
    start_buffer_read.i32 = 1;
    ret = command->command_set(nlm_buffer_start_cmd, &start_buffer_read, nlm_buffer_start_cmd->num_values);

    int32_t len = ((NLM_buffer_length + (filter_cmd->num_values - 1)) / filter_cmd->num_values) * filter_cmd->num_values;
    clog << "len = " << len << endl;
    cmd_param_t * nlm_buffer = new cmd_param_t[len];

    if(flag_buffer_get == false) // read NLModel buffer from file and write to the device
    {
        ifstream rf(filter_name, ios::out | ios::binary);
        if(!rf)
        {
            cerr << "Could not open a file " << filter_name << endl;
            exit(CONTROL_ERROR);
        }

        rf.seekg (0, rf.end);
        int32_t size = rf.tellg();
        rf.seekg (0, rf.beg);

        if(size != (NLM_buffer_length * sizeof(float)))
        {
            cerr << "NLM buffer lengths don't match" << endl;
            exit(CONTROL_DATA_LENGTH_ERROR);
        }

        // Read from file intp the nlm_buffer buffer. Will need to be done byte by byte since nlm_buffer is of type cmd_param_t
        for(int i = 0; i < NLM_buffer_length; i++)
        {
            rf.read(reinterpret_cast<char *>(&nlm_buffer[i].f), sizeof(float));
        }

        rf.peek(); // doing peek here to look for a character beyond file size so it will set eof
        rf.close();
        
        if(!rf.eof() || rf.bad())
        {
            cerr << "Error occured while reading " << filter_name << endl;
            exit(CONTROL_ERROR);
        }

        // Write the full buffer to the device
        ret = get_or_set_full_buffer(command, nlm_buffer, NLM_buffer_length, start_coeff_index_cmd, filter_cmd, flag_buffer_get);
    }
    else // Read NLModel buffer from device and write to the file
    {
        // Read the full buffer from the device
        ret = get_or_set_full_buffer(command, nlm_buffer, NLM_buffer_length, start_coeff_index_cmd, filter_cmd, flag_buffer_get);
    
        // Write filter to file
        ofstream wf(filter_name, ios::out | ios::binary);
        if(!wf)
        {
            cerr << "Could not open a file " << filter_name << endl;
            exit(CONTROL_ERROR);
        }

        for(int i = 0; i < NLM_buffer_length; i++)
        {
            wf.write(reinterpret_cast<char *>(&nlm_buffer[i].f), sizeof(float));
        }

        wf.close();
        if(wf.bad())
        {
            cerr << "Error occured when writting to " << filter_name << endl;
            exit(CONTROL_ERROR);
        }  
    }
    delete []nlm_buffer;
    return ret;
}

control_ret_t test_control_interface(Command * command, const char* out_filename)
{
    control_ret_t ret;
    cmd_t * test_cmd = command_lookup("TEST_CONTROL");
    int test_frames = 50;
    cmd_param_t * test_in_buffer = new cmd_param_t[test_cmd->num_values * test_frames];
    cmd_param_t * test_out_buffer = new cmd_param_t[test_cmd->num_values * test_frames];
    size_t num_all_vals = test_frames * test_cmd->num_values;

    string in_filename = "test_input_buf.bin";
    ifstream rf(in_filename, ios::out | ios::binary);
    if(!rf)
    {
        cerr << "Could not open a file " << in_filename << endl;
        exit(CONTROL_ERROR);
    }

    rf.seekg (0, rf.end);
    int32_t size = rf.tellg();
    rf.seekg (0, rf.beg);

    if(size != (num_all_vals * sizeof(float)))
    {
        cerr << "Test buffer lengths don't match" << endl;
        exit(CONTROL_DATA_LENGTH_ERROR);
    }

    for(int i = 0; i < num_all_vals; i++)
    {
        rf.read(reinterpret_cast<char *>(&test_in_buffer[i].ui8), sizeof(uint8_t));
    }

    rf.peek(); // doing peek here to look for a character beyond file size so it will set eof
    rf.close();

    if(!rf.eof() || rf.bad())
    {
        cerr << "Error occured while reading " << in_filename << endl;
        exit(CONTROL_ERROR);
    }

    for(int n = 0; n < test_frames; n++)
    {
        ret = command->command_set(test_cmd, &test_in_buffer[n * test_cmd->num_values], test_cmd->num_values);

        ret = command->command_get(test_cmd, &test_out_buffer[n * test_cmd->num_values], test_cmd->num_values);
    }
    delete []test_in_buffer;

    clog << "filename is " << out_filename << endl;
    ofstream wf(out_filename, ios::out | ios::binary);
    if(!wf)
    {
        cerr << "Could not open a file " << out_filename << endl;
        exit(CONTROL_ERROR);
    }

    for(int i = 0; i < num_all_vals; i++)
    {
        wf.write(reinterpret_cast<char *>(&test_out_buffer[i].ui8), sizeof(uint8_t));
    }
    delete []test_out_buffer;

    wf.close();
    if(wf.bad())
    {
        cerr << "Error occured when writing to " << out_filename << endl;
        exit(CONTROL_ERROR);
    }
    return ret;
}
