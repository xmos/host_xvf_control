# Copyright 2023 XMOS LIMITED.
# This Software is subject to the terms of the XCORE VocalFusion Licence.

import test_utils
import os
from pathlib import Path

small_cmd = "CMD_SMALL"

def test_execute_cmd_list():
    test_dir, host_bin, control_protocol = test_utils.get_dummy_files()
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

def test_version():
    test_dir, host_bin, control_protocol = test_utils.get_dummy_files()
    print("\n")

    version = test_utils.execute_command(host_bin, control_protocol, test_dir, "-v")
    changelog = Path(__file__).parents[1] / "CHANGELOG.rst"
    assert changelog.is_file(), f"Could not find {changelog}"

    with open(changelog, 'rt') as f:

        lines = f.readlines()
        version_line = 3

        print(lines[version_line])
        assert str(version[0]) == lines[version_line].strip()

