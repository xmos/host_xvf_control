// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "special_commands.hpp"
#include <fstream>

using namespace std;

control_ret_t get_or_set_full_buffer(Command * command, cmd_param_t * buffer, int32_t buffer_length, const string start_coeff_cmd_name, const string filter_cmd_name, bool flag_buffer_get)
{
    control_ret_t ret;
    cmd_t filter_cmd = {0};
    init_cmd(&filter_cmd, filter_cmd_name);
    int32_t num_filter_read_commands = (buffer_length + filter_cmd.num_values - 1) / filter_cmd.num_values;

    int32_t start_coeff = 0;
    for(int i = 0; i < num_filter_read_commands; i++)
    {
        cmd_param_t coeff;
        coeff.i32 = start_coeff;
        command->init_cmd_info(start_coeff_cmd_name);
        ret = command->command_set(&coeff);

        command->init_cmd_info(filter_cmd_name);
        if(flag_buffer_get == true) // Read from the device into the buffer
        {
            ret = command->command_get(&buffer[start_coeff]);
        }
        else // Write buffer to the device
        {
            ret = command->command_set(&buffer[start_coeff]);
        }

        start_coeff += filter_cmd.num_values;
    }
    return ret;
}

control_ret_t get_one_filter(Command * command, int32_t mic_index, int32_t far_index, string filename, uint32_t buffer_length, bool flag_buffer_get)
{
    cout << "Filename = " << filename << endl;
    
    const string start_coeff_cmd_name = "SPECIAL_CMD_AEC_FILTER_COEFF_START_OFFSET"; // Set start offset

    const string filter_cmd_name = "SPECIAL_CMD_AEC_FILTER_COEFFS"; // Get buffer cmd

    // Set start of special command sequence
    cmd_param_t far_mic_index[2];
    far_mic_index[0].i32 = far_index;
    far_mic_index[1].i32 = mic_index;
    command->init_cmd_info("SPECIAL_CMD_AEC_FAR_MIC_INDEX");
    control_ret_t ret = command->command_set(far_mic_index);
    
    // Set filter command to get it's lenght and allocate memory
    cmd_t filter_cmd = {0};
    init_cmd(&filter_cmd, filter_cmd_name);
    int32_t len = ((buffer_length + (filter_cmd.num_values - 1)) / filter_cmd.num_values) * filter_cmd.num_values;
    cmd_param_t * aec_filter = new cmd_param_t[len];

    if(flag_buffer_get == true)
    {
        // Read the full buffer from the device
        ret = get_or_set_full_buffer(command, aec_filter, buffer_length, start_coeff_cmd_name, filter_cmd_name, flag_buffer_get);

        // Write filter to file
        ofstream wf(filename, ios::out | ios::binary);
        if(!wf)
        {
            cerr << "Could not open a file " << filename << endl;
            exit(HOST_APP_ERROR);
        }

        for(uint32_t i = 0; i < buffer_length; i++)
        {
            wf.write(reinterpret_cast<char *>(&aec_filter[i].f), sizeof(float));
        }

        wf.close();
        if(wf.bad())
        {
            cerr << "Error occured when writting to " << filename << endl;
            exit(HOST_APP_ERROR);
        }
    }
    else
    {
        ifstream rf(filename, ios::out | ios::binary);
        if(!rf)
        {
            cerr << "Could not open a file " << filename << endl;
            exit(HOST_APP_ERROR);
        }

        rf.seekg (0, rf.end);
        streamoff size = rf.tellg();
        rf.seekg (0, rf.beg);

        if(size != (buffer_length * sizeof(float)))
        {
            cerr << "AEC buffer lengths don't match" << endl;
            exit(HOST_APP_ERROR);
        }

        // Read from file into aec buffer. Will need to be done byte by byte since aec_filter is of type cmd_param_t
        for(uint32_t i = 0; i < buffer_length; i++)
        {
            rf.read(reinterpret_cast<char *>(&aec_filter[i].f), sizeof(float));
        }

        rf.peek(); // doing peek here to look for a character beyond file size so it will set eof
        rf.close();

        if(!rf.eof() || rf.bad())
        {
            cerr << "Error occured while reading " << filename << endl;
            exit(HOST_APP_ERROR);
        }

        ret = get_or_set_full_buffer(command, aec_filter, buffer_length, start_coeff_cmd_name, filter_cmd_name, flag_buffer_get);
    }
    delete []aec_filter;
    return ret;
}

control_ret_t special_cmd_aec_filter(Command * command, bool flag_buffer_get, const string filename)
{
    cmd_param_t num_mics, num_farends;

    command->init_cmd_info("AEC_NUM_MICS");
    control_ret_t ret = command->command_get(&num_mics);

    command->init_cmd_info("AEC_NUM_FAREND");
    ret = command->command_get(&num_farends);

    // Get AEC filter length
    cmd_param_t filt;
    command->init_cmd_info("SPECIAL_CMD_AEC_FILTER_LENGTH");
    ret = command->command_get(&filt);

    uint32_t filter_length = filt.i32;
    cout << "AEC filter length = " << filter_length << endl;

    // Set SHF to bypass to stop AEC filter from adapting
    cmd_param_t bypass;
    bypass.ui8 = 1;
    command->init_cmd_info("SHF_BYPASS");
    command->command_set(&bypass);


    // TODO attempt to run for only one (mic, farend) pair due to timing violation this
    // command is causing in AEC. Will re-enable getting all filters once we have the chunkwise API.
    for(int far_index = 0; far_index < num_farends.i32; far_index++)
    {
        for(int mic_index = 0; mic_index < num_mics.i32; mic_index++)
        {
            // Get AEC filter for the (far_index, mic_index) pair
            string filter_name = filename + ".f" + to_string(far_index) + ".m" + to_string(mic_index) ;
            ret = get_one_filter(command, mic_index, far_index, filter_name, filter_length, flag_buffer_get);
        }
    }

    // Set AEC bypass to 0 to allow filter to adapt again
    bypass.ui8 = 0;
    command->init_cmd_info("SHF_BYPASS");
    command->command_set(&bypass);

    return ret;
}

control_ret_t special_cmd_nlmodel_buffer(Command * command, bool flag_buffer_get, const string filename)
{    
    const string start_coeff_cmd_name = "SPECIAL_CMD_NLMODEL_COEFF_START_OFFSET";

    const string filter_cmd_name = "SPECIAL_CMD_PP_NLMODEL"; // buffer cmd

    // Get buffer length
    int32_t NLM_buffer_length;
    cmd_param_t nRowCol[2];
    command->init_cmd_info("SPECIAL_CMD_PP_NLMODEL_NROW_NCOL");
    control_ret_t ret = command->command_get(nRowCol);

    string filter_name = filename + ".r" + to_string(nRowCol[0].i32) + ".c" + to_string(nRowCol[1].i32);
    cout << "Filename = " << filter_name << endl;

    NLM_buffer_length = nRowCol[0].i32 * nRowCol[1].i32;
    cout << "NLM_buffer_length = " << NLM_buffer_length << endl;

    // Set start of special command sequence
    cmd_param_t start_buffer_read;
    start_buffer_read.i32 = 1;
    command->init_cmd_info("SPECIAL_CMD_NLMODEL_START");
    ret = command->command_set(&start_buffer_read);

    // Set filter command to get it's lenght and allocate memory
    cmd_t filter_cmd = {0};
    init_cmd(&filter_cmd, filter_cmd_name);
    int32_t len = ((NLM_buffer_length + (filter_cmd.num_values - 1)) / filter_cmd.num_values) * filter_cmd.num_values;
    cout << "len = " << len << endl;
    cmd_param_t * nlm_buffer = new cmd_param_t[len];

    if(flag_buffer_get == false) // read NLModel buffer from file and write to the device
    {
        ifstream rf(filter_name, ios::out | ios::binary);
        if(!rf)
        {
            cerr << "Could not open a file " << filter_name << endl;
            exit(HOST_APP_ERROR);
        }

        rf.seekg (0, rf.end);
        streamoff size = rf.tellg();
        rf.seekg (0, rf.beg);

        if(size != (NLM_buffer_length * sizeof(float)))
        {
            cerr << "NLM buffer lengths don't match" << endl;
            exit(HOST_APP_ERROR);
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
            exit(HOST_APP_ERROR);
        }

        // Write the full buffer to the device
        ret = get_or_set_full_buffer(command, nlm_buffer, NLM_buffer_length, start_coeff_cmd_name, filter_cmd_name, flag_buffer_get);
    }
    else // Read NLModel buffer from device and write to the file
    {
        // Read the full buffer from the device
        ret = get_or_set_full_buffer(command, nlm_buffer, NLM_buffer_length, start_coeff_cmd_name, filter_cmd_name, flag_buffer_get);
    
        // Write filter to file
        ofstream wf(filter_name, ios::out | ios::binary);
        if(!wf)
        {
            cerr << "Could not open a file " << filter_name << endl;
            exit(HOST_APP_ERROR);
        }

        for(int i = 0; i < NLM_buffer_length; i++)
        {
            wf.write(reinterpret_cast<char *>(&nlm_buffer[i].f), sizeof(float));
        }

        wf.close();
        if(wf.bad())
        {
            cerr << "Error occured when writting to " << filter_name << endl;
            exit(HOST_APP_ERROR);
        }  
    }
    delete []nlm_buffer;
    return ret;
}
