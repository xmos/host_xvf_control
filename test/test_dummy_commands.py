# Copyright 2023 XMOS LIMITED.
# This Software is subject to the terms of the XCORE VocalFusion Licence.

from pathlib import Path
import os
import shutil
import subprocess
from random import randint, random
from platform import system

num_vals = 20
num_frames = 10
float_cmd = "CMD_FLOAT"
uint8_cmd = "CMD_UINT8"
control_protocol = "i2c"

system_name = system()
if system_name == "Linux":
    file_type = ".so"
elif system_name == "Darwin":
    file_type = ".dylib"
elif system_name == "Windows":
    file_type = ".dll"
else:
    assert 0, "Unsupported operating system"

host_bin = "xvf_host.exe" if system_name == "Windows" else "xvf_host"
cmd_map_so = "command_map" if system_name == "Windows" else "libcommand_map"
device_so = "device_" if system_name == "Windows" else "libdevice_"
build_dir = Path(__file__).parents[1] / "build"
test_dir = build_dir / "test"
host_bin_path = build_dir / host_bin 
host_bin_copy = test_dir / host_bin
cmd_map_dummy_path = test_dir / (cmd_map_so + "_dummy" + file_type)
cmd_map_path = test_dir / (cmd_map_so + file_type)
device_dummy_path = test_dir / (device_so + "dummy" + file_type)
device_path = test_dir / (device_so + control_protocol + file_type)

def check_files():
    assert host_bin_path.is_file() or host_bin_copy.is_file(), f"host app binary not found here {host_bin}"
    if (not host_bin_copy.is_file()) or (host_bin_path.is_file() and host_bin_copy.is_file()):
        shutil.copy2(host_bin_path,  host_bin_copy)

    assert cmd_map_dummy_path.is_file() or cmd_map_path.is_file(), f"not found {cmd_map_dummy_path}"
    if (not cmd_map_path.is_file()) or (cmd_map_dummy_path.is_file() and cmd_map_path.is_file()):
        if cmd_map_path.is_file():
            os.remove(cmd_map_path)
        os.rename(cmd_map_dummy_path, cmd_map_path)

    assert device_dummy_path.is_file() or device_path.is_file(), f"not found {device_dummy_path}"
    if (not device_path.is_file()) or (device_dummy_path.is_file() and device_path.is_file()):
        if device_path.is_file():
            os.remove(device_path)
        os.rename(device_dummy_path, device_path)

def gen_rand_array(type, min, max, size=num_vals):
    vals = []
    vals = [0 for i in range (size)]
    if type == "float":
        vals = [random() * (max - min) + min for i in range(size)]
    elif type == "int":
        vals = [randint(min, max) for i in range(size)]
    else:
        print('Unknown type: ', type)
    return vals

def run_cmd(command, verbose = False):
    result = subprocess.run(command.split(), capture_output=True, cwd=test_dir)
    if verbose or result.returncode:
        print('\n')
        print("cmd: ", result.args)
        print("returned: ", result.returncode)
        print("stdout: ", result.stdout)
        print("stderr: ", result.stderr)
    
    assert not result.returncode
    return result.stdout

def execute_command(cmd_name, cmd_vals = None):
    
    command = "./" + host_bin + " -u " + control_protocol + " " + cmd_name
    if cmd_vals != None:
        cmd_write = command + " " + ' '.join(str(val) for val in cmd_vals)
        run_cmd(cmd_write, True)

    stdout = run_cmd(command, True)

    words = str(stdout, 'utf-8').split(' ')

    # This will check that the right command is returned
    assert words[0] == cmd_name

    # Second word shuould be the value. Return as string so caller must cast to right type
    if len(words) == 2: # cmd and 1 value
        return words[1] # To avoid changing all the other tests that call execute_command and expect a single value and not an array
    else:
        return words[1:]

def single_command_test(cmd_name, cmd_vals):
    # Set and get values within the command range and test closeness
    out_list = execute_command(cmd_name, cmd_vals)

    for i in range(len(cmd_vals)):
        read_output = float(out_list[i])
        # ~20 bits should be good for this test
        rtol = 1e-6
        e = 1e-30
        abs_diff = abs(cmd_vals[i] - read_output)
        rel_error = abs(abs_diff/(cmd_vals[i] + e))
        assert rel_error < rtol

def test_dummy_commands():
    print("\n")
    check_files()

    with open('test_buf.bin', 'w'):
        pass
    
    for i in range(num_frames):
        vals = gen_rand_array('float', -2147483648, 2147483647)
        single_command_test(float_cmd, vals)

        vals = gen_rand_array('int', 0, 255)
        single_command_test(uint8_cmd, vals)
