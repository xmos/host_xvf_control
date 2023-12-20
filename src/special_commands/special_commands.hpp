// Copyright 2022-2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef SPECIAL_COMMANDS_H_
#define SPECIAL_COMMANDS_H_

#include "command.hpp"

const static opt_t options[] = {
    {"--help",                    "-h",        "display this information"                                                                       },
    {"--version",                 "-v",        "print the current version of this application",                                                 },
    {"--list-commands",           "-l",        "print list of the available commands"                                                           },
    {"--use",                     "-u",        "use specific hardware protocol, I2C, SPI and USB are available to use"                          },
    {"--command-map-path",        "-cmp",      "use specific command map path, the path is relative to the working dir"                         },
    {"--bypass-range-check",      "-br",       "bypass parameter range check",                                                                  },
    {"--dump-params",             "-d",        "print all readable parameters"                                                                  },
    {"--execute-command-list",    "-e",        "execute commands from .txt file, one command per line, don't need -u * in the .txt file"        },
    {"--get-aec-filter",          "-gf",       "get AEC filter into .bin files, default is aec_filter.bin.fx.mx"                                },
    {"--set-aec-filter",          "-sf",       "set AEC filter from .bin files, default is aec_filter.bin.fx.mx"                                },
    {"--get-nlmodel-buffer",      "-gn",       "get NLModel filter into .bin file, default is nlm_buffer.bin"                                   },
    {"--set-nlmodel-buffer",      "-sn",       "set NLModel filter from .bin file, default is nlm_buffer.bin"                                   },
    {"--test-control-interface",  "-tc",       "test control interface, default is test_buffer.bin"                                             },
    {"--test-bytestream",         "-tb",       "test device by writing a user defined stream of bytes to it"                                    },
    {"--band",                    "-b",        "NL model band to set/get (0: low-band, 1: high-band), default is 0 if unspecified"              }
};

static const size_t num_options = std::end(options) - std::begin(options);

/**
 * @brief Return the absolute path to the command map file
 *
 * If no command map path is given in the CLI argument lists,
 * the default location will be used.
 *
 * @note Will decrement argc, if option is present
 */
std::string get_cmd_map_abs_path(int * argc, char ** argv);

/**
 * @brief Gets bypass range check state by looking for --bypass-range-check in argv
 *
 * @note Will decrement argc, if option is present
 */
bool get_bypass_range_check(int * argc, char ** argv);

/**
 * @brief Gets NL model band to get/set state by looking for --band <index> in argv
 *
 * @return band index of the band to get/set NLModel for
 * @note Will decrement argc, if option is present
 */
uint8_t get_band_option(int * argc, char ** argv);

/** @brief Print application help menu */
control_ret_t print_help_menu();

/**
 * @brief Print command list loaded from the command_map
 *
 * @note Commands starting with SPECIAL_CMD_ and TEST_ will not be printed
 */
control_ret_t print_command_list();

/**
 * @brief Do a read of all possible read commands and dump it into stdout
 *
 * @param command   Pointer to the Command class object
 * @note Commands starting with SPECIAL_CMD_ and TEST_ will not be executed
 */
control_ret_t dump_params(Command * command);

/**
 * @brief Execute commands from a text file.
 *
 * Will execute one command per line.
 *
 * @param command   Pointer to the Command class object
 * @param filename  File name to read from
 * @note If filename is not specified will look for 'commands.txt'
 * @note Don't use --use inside text file
 */
control_ret_t execute_cmd_list(Command * command, const std::string = "commands.txt");

/**
 * @brief Set or get AEC filter
 *
 * @param command           Pointer to the Command class object
 * @param flag_buffer_get   Boolean to specify read/write operation
 * @param filename          File name to read from/write to
 * @note Default filename is 'aec_filter.bin'
 * @note This function will use separate files for each (mic, far-end) channel pair
 * @note So each filename will be 'aec_filter.bin.fx.mx'
 */
control_ret_t special_cmd_aec_filter(Command * command, bool flag_buffer_get, const std::string filename = "aec_filter.bin");

/**
 * @brief Set or get Non-Linear model
 *
 * @param command           Pointer to the Command class object
 * @param flag_buffer_get   Boolean to specify read/write operation
 * @param band_index        Index of the band for which the NL model is get/set
 * @param filename          File name to read from/write to
 * @note Default filename is 'nlm_buffer.bin'
 * @note This function will get number of rows and columns from the device
 * @note and expect filename to be 'nlm_buffer.bin.rx.cx'
 */
control_ret_t special_cmd_nlmodel_buffer(Command * command, bool flag_buffer_get, uint8_t band_index, const std::string filename = "nlm_buffer.bin");

/**
 * @brief Function to test control interface.
 *
 * Used for stress testing the control interface during internal device testing.
 *
 * @param command       Pointer to the Command class object
 * @param filename      File name to use for an output buffer
 */
control_ret_t test_control_interface(Command * command, const std::string filename = "test_buffer.bin");

/**
 * @brief Function to test the device control interface error handling
 *
 * This option is used only for internal device testing. It writes a stream of data that it
 * reads from a file to the device by using the low level command get and set functions.
 *
 * @param command       Pointer to the Command class object
 * @param filename      File name containing the input byte stream
 */
control_ret_t test_bytestream(Command * command, const std::string in_filename);

#endif
