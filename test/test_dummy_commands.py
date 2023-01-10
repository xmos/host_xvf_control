# Copyright 2023 XMOS LIMITED.
# This Software is subject to the terms of the XCORE VocalFusion Licence.

from pathlib import Path
import os
import shutil
import subprocess
import numpy as np

num_vals = 20
num_frames = 10
float_cmd = "CMD_FLOAT"
uint8_cmd = "CMD_UINT8"

host_bin = "xvf_hostapp"
build_dir = "../build/"
test_dir = "../build/test/"
host_bin_path = build_dir + host_bin
host_bin_copy = test_dir + host_bin
cmd_map_dummy_path = test_dir + "libcommand_map_dummy.so"
cmd_map_path = test_dir + "libcommand_map.so"
device_dummy_path = test_dir + "libdevice_dummy.so"
device_path = test_dir + "libdevice_i2c.so"

def check_files():
    assert Path(host_bin_path).is_file() or Path(host_bin_copy).is_file(), f"host app binary not found here {host_bin}"
    if (not Path(host_bin_copy).is_file()) or (Path(host_bin_path).is_file() and Path(host_bin_copy).is_file()):
        shutil.copy2(host_bin_path,  host_bin_copy)

    assert Path(cmd_map_dummy_path).is_file() or Path(cmd_map_path).is_file(), f"not found {cmd_map_dummy_path}"
    if (not Path(cmd_map_path).is_file()) or (Path(cmd_map_dummy_path).is_file() and Path(cmd_map_path).is_file()):
        os.rename(cmd_map_dummy_path, cmd_map_path)

    assert Path(device_dummy_path).is_file() or Path(device_path).is_file(), f"not found {device_dummy_path}"
    if (not Path(device_path).is_file()) or (Path(device_dummy_path).is_file() and Path(device_path).is_file()):
        os.rename(device_dummy_path, device_path)

def gen_rand_array(type, min, max, size=num_vals):
    if type == "float":
        vals = np.random.rand(size) * (max - min) + min
    elif type == "int":
        vals = np.random.randint(min, max+1, size, dtype = np.int32)
    else:
        print('Unknown type: ', type)
    return vals

def run_cmd(command, verbose = False):
    result = subprocess.run(command.split(), capture_output=True)
    if verbose or result.returncode:
        print('\n')
        print("cmd: ", result.args)
        print("returned: ", result.returncode)
        print("stdout: ", result.stdout)
        print("stderr: ", result.stderr)
    
    assert not result.returncode
    return result.stdout

def execute_command(cmd_name, cmd_vals = None, protocol = "i2c"):
    
    command = "sudo ./" + host_bin + " -u " + protocol + " " + cmd_name
    if cmd_vals.all() != None:
        cmd_write = command + " " + ' '.join(str(val) for val in cmd_vals)
        run_cmd(cmd_write, True)

    stdout = run_cmd(command, True)

    words = str(stdout, 'utf-8').split(' ')
    print(words)

    # This will check that the right command is returned
    assert words[0] == cmd_name

    # Second word shuould be the value. Return as string so caller must cast to right type
    if len(words) == 2: # cmd and 1 value
        return words[1] # To avoid changing all the other tests that call execute_command and expect a single value and not an array
    else:
        return words[1:]

def single_command_test(cmd_name, cmd_vals, protocol = 'i2c'):
    # Set and get values within the command range and test closeness
    out_list = execute_command(cmd_name, cmd_vals, protocol)

    for i in range(len(cmd_vals)):
        read_output = float(out_list[i])
        assert np.isclose(read_output, cmd_vals[i])

def test_dummy_commands():
    print("\n")
    check_files()

    current_dir = os.getcwd()
    os.chdir(test_dir)
    
    vals = gen_rand_array('float', np.iinfo(np.int32).min, np.iinfo(np.int32).max)
    single_command_test(float_cmd, vals)

    os.chdir(current_dir)
