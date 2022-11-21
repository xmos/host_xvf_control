// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "special_commands.hpp"
#include <cassert>
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

using namespace std;

opt_t options[] = {
            {"--help",                 "-h", "print this options menu",          0},
            {"--list-commands",        "-l", "print the list of commands",       0},
            {"--vendor-id",            "-v", "set USB Vendor ID",                1},
            {"--product-id",           "-p", "set USB Product ID",               1},
            {"--dump-params",          "-d", "print all the parameters",         0},
            {"--skip-version-check",   "-s", "skip version check",               0},
            {"--execute-command-list", "-e", "execute commands from .txt file",  1},
            {"--get-aec-filter",       "--get-aec-filter", "Get AEC filter", 0}, // TODO pass filename as an option
            {"--get-nlmodel-buffer",   "--get-nlmodel-buffer", "Get NLModel filter", 0},
            {"--set-nlmodel-buffer",   "--set-nlmodel-buffer", "Set NLModel filter", 0},
};

opt_t * option_lookup(const string str)
{
    for(int i = 0; i < ARRAY_SIZE(options); i++)
    {
        opt_t * opt = &options[i];
        if ((str == opt->long_name) || (str == opt->short_name))
        {
            return opt;
        }
    }
    return nullptr;
}

cmd_t * command_lookup(const string str, cmd_t * commands, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        cmd_t * cmd = &commands[i];
        if (str == cmd->cmd_name)
        {
            return cmd;
        }
    }
    return nullptr;
}

control_ret_t print_options_list(void)
{
    for(opt_t opt : options)
    {
        cout << "Use " << opt.short_name <<  " or " << opt.long_name
        << " to " << opt.info << "." << endl;
    }
    return CONTROL_SUCCESS;
}

control_ret_t print_command_list(cmd_t * commands, size_t size)
{
    for(size_t i = 0; i < size; i ++)
    {
        cmd_t * cmd = &commands[i];
        cout << cmd->cmd_name << " is " << command_rw_type_name(cmd->rw)
        << " and requires/returns " << cmd->num_values
        << " values of type " << command_param_type_name(cmd->type)
        << ". "<< cmd->info << "." << endl;
    }
    return CONTROL_SUCCESS;
}

control_ret_t dump_params(cmd_t * commands, size_t size)
{
    control_ret_t ret = CONTROL_ERROR;
    Command command;
    for(size_t i = 0; i < size; i ++)
    {
        ret = command.do_command(&commands[i], nullptr, 0);
    }
    return ret;
}

void assert_on_cmd_error(cmd_t *cmd, control_ret_t ret)
{
    if(ret != CONTROL_SUCCESS)
    {
        printf("%s command returns error %d\n", cmd->cmd_name.c_str(), ret);
        assert(0);
    }
}

void get_or_set_full_buffer(cmd_param_t *buffer, int32_t buffer_length, cmd_t *start_coeff_index_cmd, cmd_t *filter_cmd, int flag_buffer_get)
{
    Command command;
    control_ret_t ret;
    int32_t num_filter_read_commands = buffer_length / filter_cmd->num_values;
    int32_t last_remaining_coeffs = buffer_length - (num_filter_read_commands*filter_cmd->num_values);
    if(last_remaining_coeffs)
    {
        num_filter_read_commands += 1;
    }

    int32_t start_coeff = 0;
    unsigned backup_num_values = filter_cmd->num_values; // Save a copy of num_values for the get_filter cmd in case we overwrite it for the last chunk
    for(int i=0; i<num_filter_read_commands; i++)
    {
        //printf("reading from start_coeff %d\n", start_coeff);
        if((i == (num_filter_read_commands-1)) && (last_remaining_coeffs))
        {
            filter_cmd->num_values = last_remaining_coeffs;
        }
        cmd_param_t coeff;
        coeff.i32 = start_coeff;
        ret = command.command_set(start_coeff_index_cmd, &coeff, 1);
        assert_on_cmd_error(start_coeff_index_cmd, ret);

        if(flag_buffer_get) // Read from the device into the buffer
        {
            ret = command.command_get(filter_cmd, &buffer[start_coeff], filter_cmd->num_values);
            assert_on_cmd_error(filter_cmd, ret);
        }
        else // Write buffer to the device
        {
            ret = command.command_set(filter_cmd, &buffer[start_coeff], filter_cmd->num_values);
            assert_on_cmd_error(filter_cmd, ret);
        }

        start_coeff += filter_cmd->num_values;
    }
    filter_cmd->num_values = backup_num_values; 
}

void get_one_filter(int32_t mic_index, int32_t far_index, std::string filename, uint32_t buffer_length, cmd_t * commands, size_t num_commands)
{
    Command command;
    control_ret_t ret;
    printf("filename = %s\n", filename.c_str());

    cmd_t *far_mic_index_cmd = command_lookup("SPECIAL_CMD_AEC_FAR_MIC_INDEX", commands, num_commands); // Start cmd
    assert(far_mic_index_cmd != NULL);
    
    cmd_t *start_coeff_index_cmd = command_lookup("SPECIAL_CMD_AEC_FILTER_COEFF_START_OFFSET", commands, num_commands); // Set start offset
    assert(start_coeff_index_cmd != NULL);

    cmd_t *filter_cmd = command_lookup("SPECIAL_CMD_AEC_FILTER_COEFFS", commands, num_commands); // Get buffer cmd
    assert(filter_cmd != NULL);

    // Set start of special command sequence
    cmd_param_t far_mic_index[2];
    far_mic_index[0].i32 = far_index;
    far_mic_index[1].i32 = mic_index;
    ret = command.command_set(far_mic_index_cmd, far_mic_index, far_mic_index_cmd->num_values);
    assert_on_cmd_error(far_mic_index_cmd, ret);

    cmd_param_t *aec_filter = new cmd_param_t[buffer_length];

    // Read the full buffer from the device
    get_or_set_full_buffer(aec_filter, buffer_length, start_coeff_index_cmd, filter_cmd, 1);
    
    // Write filter to file
    FILE *fp = fopen(filename.c_str(), "wb");
    for(int i=0; i<buffer_length; i++)
    {
        fwrite(&aec_filter[i].f, sizeof(float), 1, fp); // It's annoying having to write byte by byte
    }
    fclose(fp);
    delete []aec_filter;
}

control_ret_t get_aec_filter(const char *filename, cmd_t * commands, size_t num_commands)
{
    Command command;
    command.init_device(); // Initialise the device

    printf("In get_aec_filter()\n");
    control_ret_t ret;
    cmd_t *num_mics_cmd = command_lookup("AEC_NUM_MICS", commands, num_commands);
    assert(num_mics_cmd != NULL);

    cmd_t *num_farends_cmd = command_lookup("AEC_NUM_FARENDS", commands, num_commands);
    assert(num_farends_cmd != NULL);
    cmd_param_t num_mics, num_farends;

    ret = command.command_get(num_mics_cmd, &num_mics, num_mics_cmd->num_values);
    assert_on_cmd_error(num_mics_cmd, ret);

    ret = command.command_get(num_farends_cmd, &num_farends, num_farends_cmd->num_values);
    assert_on_cmd_error(num_farends_cmd, ret);

     // Get AEC filter length
    cmd_t *aec_filter_length_cmd = command_lookup("SPECIAL_CMD_AEC_FILTER_LENGTH", commands, num_commands);
    assert(aec_filter_length_cmd != NULL);
    cmd_param_t filt;
    ret = command.command_get(aec_filter_length_cmd, &filt, aec_filter_length_cmd->num_values);
    assert_on_cmd_error(aec_filter_length_cmd, ret);

    uint32_t filter_length = filt.i32;
    printf("AEC filter length = %d\n", filter_length);

    // TODO attempt to run for only one (mic, farend) pair due to timing violation this
    // command is causing in AEC. Will re-enable getting all filters once we have the chunkwise API.
    for(int mic_index=0; mic_index<1/*num_mics.i32*/; mic_index++)
    {
        for(int far_index=0; far_index<1/*num_farends.i32*/; far_index++)
        {
            // Get AEC filter for the (mic_index, far_index) pair
            std::string filter_name = filename;
            filter_name = filter_name + "_m" + std::to_string(mic_index) + "_f" + std::to_string(far_index) + ".bin";
            get_one_filter(mic_index, far_index, filter_name, filter_length, commands, num_commands);
        }
    }
    return CONTROL_SUCCESS;
}

control_ret_t special_cmd_nlmodel_buffer(const char* filename, int32_t flag_buffer_get, cmd_t * commands, size_t num_commands)
{
    Command command;
    command.init_device(); // Initialise the device
    printf("In special_cmd_nlmodel_buffer()\n");
    control_ret_t ret;
    printf("filename = %s, flag_buffer_get = %d\n", filename, flag_buffer_get);

    cmd_t *nlm_buffer_start_command = command_lookup("SPECIAL_CMD_NLMODEL_START", commands, num_commands); // Start cmd
    assert(nlm_buffer_start_command != NULL);
    
    cmd_t *nlm_buffer_length_cmd = command_lookup("SPECIAL_CMD_PP_NLMODEL_NROW_NCOL", commands, num_commands); // Get buffer length
    assert(nlm_buffer_length_cmd != NULL);

    cmd_t *start_coeff_index_cmd = command_lookup("SPECIAL_CMD_NLMODEL_COEFF_START_OFFSET", commands, num_commands); // Set start offset
    assert(start_coeff_index_cmd != NULL);

    cmd_t *filter_cmd = command_lookup("SPECIAL_CMD_PP_NLMODEL", commands, num_commands); // buffer cmd
    assert(filter_cmd != NULL);

    // Get buffer length
    int32_t NLM_buffer_length;
    cmd_param_t nRowCol[2];
    ret = command.command_get(nlm_buffer_length_cmd, nRowCol, nlm_buffer_length_cmd->num_values);
    assert_on_cmd_error(nlm_buffer_length_cmd, ret);

    NLM_buffer_length = nRowCol[0].i32 * nRowCol[1].i32;
    printf("NLM_buffer_length = %d\n", NLM_buffer_length);

    // Set start of special command sequence
    cmd_param_t start_buffer_read;
    start_buffer_read.i32 = 1;
    ret = command.command_set(nlm_buffer_start_command, &start_buffer_read, nlm_buffer_start_command->num_values);
    assert_on_cmd_error(nlm_buffer_start_command, ret);

    cmd_param_t *nlm_buffer = new cmd_param_t[NLM_buffer_length];

    std::string fname = filename;
    //fname = fname + ".bin";
    FILE *fp;
    if(!flag_buffer_get) // read NLModel buffer from file and write to the device
    {
        fp = fopen(fname.c_str(), "rb");
        fseek(fp, 0, SEEK_END);
        int32_t size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        assert(size == (NLM_buffer_length*sizeof(float)));

        // Read from file intp the nlm_buffer buffer. Will need to be done byte by byte since nlm_buffer is of type cmd_param_t
        for(int i=0; i<NLM_buffer_length; i++)
        {
            fread(&nlm_buffer[i].f, sizeof(float), 1, fp);
        }
        // Write the full buffer to the device
        get_or_set_full_buffer(nlm_buffer, NLM_buffer_length, start_coeff_index_cmd, filter_cmd, 0);
    }
    else // Read NLModel buffer from device and write to the file
    {
        // Read the full buffer from the device
        get_or_set_full_buffer(nlm_buffer, NLM_buffer_length, start_coeff_index_cmd, filter_cmd, 1);
    
        // Write filter to file
        fp = fopen(fname.c_str(), "wb");
        float rows = (float)nRowCol[0].i32;
        float cols = (float)nRowCol[1].i32;
        // Write the number of rows and number of columns as the first 2*sizeof(int32_t) bytes in the file
        fwrite(&rows, sizeof(float), 1, fp); // Write as float since rest of the data is also in float.
        fwrite(&cols, sizeof(float), 1, fp); // Write as float since rest of the data is also in float.

        for(int i=0; i<NLM_buffer_length; i++)
        {
            fwrite(&nlm_buffer[i].f, sizeof(float), 1, fp); // It's annoying having to write byte by byte
        }   
    }
    fclose(fp);
    delete []nlm_buffer;
    return CONTROL_SUCCESS;
}
