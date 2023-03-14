# Copyright 2023 XMOS LIMITED.
# This Software is subject to the terms of the XCORE VocalFusion Licence.

import test_utils
import os
from pathlib import Path

small_cmd = "CMD_SMALL"

def test_execute_cmd_list():
    test_dir, host_bin, control_protocol, cmd_map_name = test_utils.get_dummy_files()
    print("\n")

    cmd_list_path = test_dir / "commands.txt"
    if cmd_list_path.is_file(): os.remove(cmd_list_path)
    with open(cmd_list_path, "w") as f:
        f.write(small_cmd + " 156   -894564     4586543\n")
        f.write("\n")
        f.write(small_cmd)

    # check that it does not fail
    out = test_utils.execute_command(host_bin, control_protocol, test_dir, "-e")
    print(out)
    print(f"cmd_map_name in test_execute_cmd_list() is {cmd_map_name}")
    os.system("tree")
    out = test_utils.execute_command(host_bin, control_protocol, test_dir, "-e", cmd_map_path= str(Path("build/test") / cmd_map_name) )
    print(out)


def test_version():
    test_dir, host_bin, control_protocol, _ = test_utils.get_dummy_files()
    print("\n")

    version = test_utils.execute_command(host_bin, control_protocol, test_dir, "-v")
    changelog = Path(__file__).parents[1] / "CHANGELOG.rst"
    assert changelog.is_file(), f"Could not find {changelog}"

    with open(changelog, 'rt') as f:

        lines = f.readlines()
        version_line = 3

        print(lines[version_line])
        assert str(version[0]) == lines[version_line].strip()

def test_range_check():
    test_dir, host_bin, control_protocol, _ = test_utils.get_dummy_files()
    for i in range(10):
        # this part of the test will check if the range check works as expected
        vals = test_utils.gen_rand_array('int', 0, 6, 1)
        if ((vals[0] >= 0) and (vals[0] <= 3)) or (vals[0] == 5):
            test_utils.execute_command(host_bin, control_protocol, test_dir, "RANGE_TEST0", cmd_vals=vals, expect_success=True)
        else:
            test_utils.execute_command(host_bin, control_protocol, test_dir, "RANGE_TEST0", cmd_vals=vals, expect_success=False)

        vals = test_utils.gen_rand_array('float', 0.0, 1.1, 3)
        if((vals[0] >= 0.0) and (vals[0] <= 1.0)) and ((vals[1] >= 0.5) and (vals[1] <= 1.0)) and ((vals[2] >= 0.0) and (vals[2] <= 0.5)):
            test_utils.execute_command(host_bin, control_protocol, test_dir, "RANGE_TEST1", cmd_vals=vals, expect_success=True)
        else:
            test_utils.execute_command(host_bin, control_protocol, test_dir, "RANGE_TEST1", cmd_vals=vals, expect_success=False)

        vals = test_utils.gen_rand_array('int', 0, 15, 2)
        if((vals[0] >= 0) and (vals[0] <= 3)) and (((vals[1] >= 0) and (vals[1] <= 6)) or ((vals[1] >= 10) and (vals[1] <= 14))):
            test_utils.execute_command(host_bin, control_protocol, test_dir, "RANGE_TEST2", cmd_vals=vals, expect_success=True)
        else:
            test_utils.execute_command(host_bin, control_protocol, test_dir, "RANGE_TEST2", cmd_vals=vals, expect_success=False)

        vals = test_utils.gen_rand_array('int', 0, 400, 3)
        if((vals[0] >= 0) and (vals[0] <= 255)) and ((vals[2] >= 0) and (vals[2] <= 255)):
            test_utils.execute_command(host_bin, control_protocol, test_dir, "RANGE_TEST3", cmd_vals=vals, expect_success=True)
        else:
            test_utils.execute_command(host_bin, control_protocol, test_dir, "RANGE_TEST3", cmd_vals=vals, expect_success=False)

        # here we check that bypass works
        vals = test_utils.gen_rand_array('int', -2147483648, 2147483647, 1)
        test_utils.execute_command(host_bin, control_protocol, test_dir, "-br RANGE_TEST0", cmd_vals=vals, expect_success=True)

        vals = test_utils.gen_rand_array('float', -2147483648, 2147483647, 3)
        test_utils.execute_command(host_bin, control_protocol, test_dir, "-br RANGE_TEST1", cmd_vals=vals, expect_success=True)

        vals = test_utils.gen_rand_array('int', 0, 255, 2)
        test_utils.execute_command(host_bin, control_protocol, test_dir, "-br RANGE_TEST2", cmd_vals=vals, expect_success=True)

        vals = test_utils.gen_rand_array('int', 0, 4294967295, 3)
        test_utils.execute_command(host_bin, control_protocol, test_dir, "-br RANGE_TEST3", cmd_vals=vals, expect_success=True)
