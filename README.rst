===========================
HOST_XVF_CONTROL Repository
===========================

HOST_XVF_CONTROL is host control reference application to be used for XVF3800 and onwards.

*******
Cloning
*******

Some dependent components are included as git sub modules. These can be obtained by cloning this repository with the following command:

    git clone --recurse-submodules git@github.com:xmos/sw_xvf_host.git

********
Building
********

Build with cmake:

.. tab:: Linux and Mac

    .. code-block:: console

        mkdir build && cd build && cmake -S.. && make

.. tab:: Windows

    .. code-block:: console

        # with VS tools
        mkdir build && cd build && cmake -G "NMake Makefiles" -S .. && nmake

*****
Using
*****

In order to use the application you should have the following files in the same location

    xvf_host(.exe)
    (lib)command_map.(so/dll/dylib)
    (lib)device_{protocol}.(so/dll/dylib)

The application and the device drivers can be obtained by following the build instructions of this repo. Command map is a part of the firmware code and built separately.
To find out use cases and more information about the application use:

    ./xvf_host --help

*****************************************
Supported platforms and control protocols
*****************************************

- Raspberry Pi
    - xvf_host
    - libdevice_i2c.so
    - libdevice_spi.so
- Mac x86
    - xvf_host
- Windows
    - xvf_host.exe

Note that Mac and Windows builds don't have hardware drivers for now. If you want to test Mac or Windows applications,
use ``-DTESTING=ON`` option when you do cmake, it will build dummy device and command_map shared objects for all platforms.
