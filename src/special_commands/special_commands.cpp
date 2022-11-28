// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#include "special_commands.hpp"
#include "dlfcn.h"
#include <cassert>
#include <iomanip>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

using namespace std;

opt_t options[] = {
            {"--help",                 "-h",          "display this information",            "0"                                        },
            {"--list-commands",        "-l",          "print the list of commands",          "0"                                        },
            {"--vendor-id",            "-v",          "set USB Vendor ID",                   "0"                                        },
            {"--product-id",           "-p",          "set USB Product ID",                  "0"                                        },
            {"--dump-params",          "-d",          "print all the parameters",            "0"                                        },
            {"--skip-version-check",   "-s",          "skip version check",                  "0"                                        },
            {"--execute-command-list", "-e",          "execute commands from .txt file",     "0"                                        },
            {"--use",                  "-u",          "use specific harware protocol,",      "I2C and SPI are available to use"         },
            {"--get-aec-filter",       "-gF",         "get AEC filter into .bin files,",     "default is aec_filter.bin.fx.mx"          },
            {"--set-aec-filter",       "-sF",         "set AEC filter from .bin files,",     "default is aec_filter.bin.fx.mx"          },
            {"--get-nlmodel-buffer",   "-gN",         "get NLModel filter into .bin file,",  "default is nlm_buffer.bin"                },
            {"--set-nlmodel-buffer",   "-sN",         "set NLModel filter from .bin file,",  "default is nlm_buffer.bin"                }
};

cmd_t * commands;
size_t num_commands;

void load_command_map_dll()
{
    string dyn_lib_path = get_dynamic_lib_path("/libcommand_map");
    void * sofile = dlopen(dyn_lib_path.c_str(), RTLD_NOW);
    if(!sofile)
    {
        printf("%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    cmd_t* (*get_command_map)();
    get_command_map = (cmd_t* (*)())dlsym(sofile, "get_command_map");

    uint32_t (*get_num_commands)();
    get_num_commands = (uint32_t (*)())dlsym(sofile, "get_num_commands");

    commands = get_command_map();
    num_commands = get_num_commands();
}

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

cmd_t * command_lookup(const string str)
{
    for (size_t i = 0; i < num_commands; i++)
    {
        cmd_t * cmd = &commands[i];
        if (str == cmd->cmd_name)
        {
            return cmd;
        }
    }
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

    cout << "usage: xvf_hostapp_rpi [ -h | --help ]" << endl
    << setw(47) << "[ -l | --list-commands ]" << endl
    << setw(68) << "[ -u | --use <protocol>] [ command | option ]" << endl
    << "Options:" << endl; 
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
    for(size_t i = 0; i < num_commands; i ++)
    {
        cmd_t * cmd = &commands[i];
        cout << cmd->cmd_name << " is " << command_rw_type_name(cmd->rw)
        << " and requires/returns " << cmd->num_values
        << " values of type " << command_param_type_name(cmd->type)
        << ". "<< cmd->info << "." << endl;
    }
    return CONTROL_SUCCESS;
}

control_ret_t dump_params(Command * command)
{
    control_ret_t ret = CONTROL_ERROR;
    for(size_t i = 0; i < num_commands; i ++)
    {
        ret = command->do_command(&commands[i], nullptr, 0, 0);
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

void get_or_set_full_buffer(Command * command, cmd_param_t *buffer, int32_t buffer_length, cmd_t *start_coeff_index_cmd, cmd_t *filter_cmd, bool flag_buffer_get)
{
    control_ret_t ret;
    int32_t num_filter_read_commands = (buffer_length + filter_cmd->num_values - 1) / filter_cmd->num_values;

    int32_t start_coeff = 0;
    for(int i=0; i<num_filter_read_commands; i++)
    {
        cmd_param_t coeff;
        coeff.i32 = start_coeff;
        ret = command->command_set(start_coeff_index_cmd, &coeff, 1);
        assert_on_cmd_error(start_coeff_index_cmd, ret);

        //printf("start coeff = %d, num_values = %d\n", start_coeff, filter_cmd->num_values);
        if(flag_buffer_get == true) // Read from the device into the buffer
        {
            ret = command->command_get(filter_cmd, &buffer[start_coeff], filter_cmd->num_values);
            assert_on_cmd_error(filter_cmd, ret);
        }
        else // Write buffer to the device
        {
            ret = command->command_set(filter_cmd, &buffer[start_coeff], filter_cmd->num_values);
            assert_on_cmd_error(filter_cmd, ret);
        }

        start_coeff += filter_cmd->num_values;
    }
}

void get_one_filter(Command * command, int32_t mic_index, int32_t far_index, string filename, uint32_t buffer_length, bool flag_buffer_get)
{
    control_ret_t ret;
    printf("filename = %s\n", filename.c_str());

    cmd_t *far_mic_index_cmd = command_lookup("SPECIAL_CMD_AEC_FAR_MIC_INDEX"); // Start cmd
    assert(far_mic_index_cmd != nullptr);
    
    cmd_t *start_coeff_index_cmd = command_lookup("SPECIAL_CMD_AEC_FILTER_COEFF_START_OFFSET"); // Set start offset
    assert(start_coeff_index_cmd != nullptr);

    cmd_t *filter_cmd = command_lookup("SPECIAL_CMD_AEC_FILTER_COEFFS"); // Get buffer cmd
    assert(filter_cmd != nullptr);

    // Set start of special command sequence
    cmd_param_t far_mic_index[2];
    far_mic_index[0].i32 = far_index;
    far_mic_index[1].i32 = mic_index;
    ret = command->command_set(far_mic_index_cmd, far_mic_index, far_mic_index_cmd->num_values);
    assert_on_cmd_error(far_mic_index_cmd, ret);
    
    int32_t len = ((buffer_length + (filter_cmd->num_values - 1))/filter_cmd->num_values) * filter_cmd->num_values;
    cmd_param_t *aec_filter = new cmd_param_t[len];
    FILE * fp; 
    if(flag_buffer_get == true)
    {
        // Read the full buffer from the device
        get_or_set_full_buffer(command, aec_filter, buffer_length, start_coeff_index_cmd, filter_cmd, flag_buffer_get);

        // Write filter to file
        if((fp = fopen(filename.c_str(), "wb")) == NULL)
        {
            cout << "Failed to open " << filename << endl;
            exit(CONTROL_ERROR);
        }
        for(int i=0; i<buffer_length; i++)
        {
            fwrite(&aec_filter[i].f, sizeof(float), 1, fp); // It's annoying having to write byte by byte
        }
    }
    else
    {
        if((fp = fopen(filename.c_str(), "rb")) == NULL)
        {
            cout << "Failed to open " << filename << endl;
            exit(CONTROL_ERROR);
        }
        fseek(fp, 0, SEEK_END);
        int32_t size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        printf("Writing filter of size %d\n",size);

        // Read from file intp the nlm_buffer buffer. Will need to be done byte by byte since nlm_buffer is of type cmd_param_t
        for(int i=0; i<size; i++)
        {
            fread(&aec_filter[i].f, sizeof(float), 1, fp);
        }
        get_or_set_full_buffer(command, aec_filter, buffer_length, start_coeff_index_cmd, filter_cmd, flag_buffer_get);
    }
    fclose(fp);
    delete []aec_filter;
}

control_ret_t special_cmd_aec_filter(Command * command, bool flag_buffer_get, const char *filename)
{
    command->init_device(); // Initialise the device

    printf("In special_cmd_aec_filter()\n");
    control_ret_t ret;
    cmd_t *num_mics_cmd = command_lookup("AEC_NUM_MICS");
    assert(num_mics_cmd != nullptr);

    cmd_t *num_farends_cmd = command_lookup("AEC_NUM_FARENDS");
    assert(num_farends_cmd != nullptr);
    cmd_param_t num_mics, num_farends;

    ret = command->command_get(num_mics_cmd, &num_mics, num_mics_cmd->num_values);
    assert_on_cmd_error(num_mics_cmd, ret);

    ret = command->command_get(num_farends_cmd, &num_farends, num_farends_cmd->num_values);
    assert_on_cmd_error(num_farends_cmd, ret);

     // Get AEC filter length
    cmd_t *aec_filter_length_cmd = command_lookup("SPECIAL_CMD_AEC_FILTER_LENGTH");
    assert(aec_filter_length_cmd != nullptr);
    cmd_param_t filt;
    ret = command->command_get(aec_filter_length_cmd, &filt, aec_filter_length_cmd->num_values);
    assert_on_cmd_error(aec_filter_length_cmd, ret);

    uint32_t filter_length = filt.i32;
    printf("AEC filter length = %d\n", filter_length);

    // TODO attempt to run for only one (mic, farend) pair due to timing violation this
    // command is causing in AEC. Will re-enable getting all filters once we have the chunkwise API.
    for(int far_index=0; far_index<num_farends.i32; far_index++)
    {
        for(int mic_index=0; mic_index<num_mics.i32; mic_index++)
        {
            // Get AEC filter for the (far_index, mic_index) pair
            string filter_name = filename;
            filter_name = filter_name + ".f" + to_string(far_index) + ".m" + to_string(mic_index) ;
            get_one_filter(command, mic_index, far_index, filter_name, filter_length, flag_buffer_get);
        }
    }
    return CONTROL_SUCCESS;
}

control_ret_t special_cmd_nlmodel_buffer(Command * command, bool flag_buffer_get, const char* filename)
{
    command->init_device(); // Initialise the device
    printf("In special_cmd_nlmodel_buffer()\n");
    control_ret_t ret;
    printf("filename = %s, flag_buffer_get = %d\n", filename, flag_buffer_get);

    cmd_t *nlm_buffer_start_command = command_lookup("SPECIAL_CMD_NLMODEL_START"); // Start cmd
    assert(nlm_buffer_start_command != nullptr);
    
    cmd_t *nlm_buffer_length_cmd = command_lookup("SPECIAL_CMD_PP_NLMODEL_NROW_NCOL"); // Get buffer length
    assert(nlm_buffer_length_cmd != nullptr);

    cmd_t *start_coeff_index_cmd = command_lookup("SPECIAL_CMD_NLMODEL_COEFF_START_OFFSET"); // Set start offset
    assert(start_coeff_index_cmd != nullptr);

    cmd_t *filter_cmd = command_lookup("SPECIAL_CMD_PP_NLMODEL"); // buffer cmd
    assert(filter_cmd != nullptr);

    // Get buffer length
    int32_t NLM_buffer_length;
    cmd_param_t nRowCol[2];
    ret = command->command_get(nlm_buffer_length_cmd, nRowCol, nlm_buffer_length_cmd->num_values);
    assert_on_cmd_error(nlm_buffer_length_cmd, ret);

    NLM_buffer_length = nRowCol[0].i32 * nRowCol[1].i32;
    printf("NLM_buffer_length = %d\n", NLM_buffer_length);

    // Set start of special command sequence
    cmd_param_t start_buffer_read;
    start_buffer_read.i32 = 1;
    ret = command->command_set(nlm_buffer_start_command, &start_buffer_read, nlm_buffer_start_command->num_values);
    assert_on_cmd_error(nlm_buffer_start_command, ret);

    int32_t len = ((NLM_buffer_length + (filter_cmd->num_values - 1))/filter_cmd->num_values) * filter_cmd->num_values;
    printf("len = %d\n",len);
    cmd_param_t *nlm_buffer = new cmd_param_t[len];

    string fname = filename;
    FILE *fp;
    if(flag_buffer_get == false) // read NLModel buffer from file and write to the device
    {
        if((fp = fopen(filename, "rb")) == NULL)
        {
            cout << "Failed to open " << filename << endl;
            exit(CONTROL_ERROR);
        }
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
        get_or_set_full_buffer(command, nlm_buffer, NLM_buffer_length, start_coeff_index_cmd, filter_cmd, flag_buffer_get);
    }
    else // Read NLModel buffer from device and write to the file
    {
        // Read the full buffer from the device
        get_or_set_full_buffer(command, nlm_buffer, NLM_buffer_length, start_coeff_index_cmd, filter_cmd, flag_buffer_get);
    
        // Write filter to file
        if((fp = fopen(filename, "wb")) == NULL)
        {
            cout << "Failed to open " << filename << endl;
            exit(CONTROL_ERROR);
        }
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
