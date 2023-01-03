# HOST_XVF_CONTROL Repository

HOST_XVF_CONTROL is host control reference application to be used for XVF3800 and onwards.

## Cloning

Some dependent components are included as git sub modules. These can be obtained by cloning this repository with the following command:

    git clone --recurse-submodules git@github.com:xmos/sw_xvf_host.git

## Building

Build with cmake:

    mkdir build && cd build && cmake -S.. && make

## Using

In order to use the application you should have the following files in the same location

    xvf_hostapp_{platform}
    libcommand_map.(so/dll/dylib)
    libdevice_{platform}_{protocol}.(so/dll/dylib)

The application and the device drivers can be obtained by following the build instructions of this repo. Command map is a part of the firmware code and built separately.
To find out use cases and more information about the application use:

    ./xvf_hostapp_{platform} --help
