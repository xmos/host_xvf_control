===========================
HOST_XVF_CONTROL Repository
===========================

HOST_XVF_CONTROL is a host control and DFU reference application.
It can be used with products in the XVF38xx family.

*******
Cloning
*******

Some dependent components are included as git submodules. These can be obtained by cloning this repository with the following command:

.. code-block:: console

    git clone --recurse-submodules git@github.com:xmos/xvf_host_control.git

************
Requirements
************

- CMake 3.13 or later
- Visual Studio 2022 Tools (Windows only)
- Ninja (Windows only)

********
Building
********

Build with CMake from the host_xvf_control/ folder:

- on Linux and Mac

.. code-block:: console

    cmake -B build && cd build && make

- on Windows

.. code-block:: console

    # building with VS tools
    cmake -G Ninja -B build && cd build && ninja

.. note::

    Windows drivers can only be built with 32-bit tools.

.. note::

    Windows drivers are currently supported only for *Visual Studio 2022 Tools*. If a different toolchain is required, the static *libusb* library should be built and linked manually. More details can be found in fwk_rtos//modules/sw_services/device_control/host/libusb/Win32/README.md.

*****
Using
*****

In order to use the host control application place the following files in the same location:

- xvf_host(.exe)
- (lib)command_map.(so/dll/dylib)
- (lib)device_{protocol}.(so/dll/dylib)

.. note::

    - Linux dynamic libraries end with ``.so``
    - Apple dynamic libraries end with ``.dylib``
    - Windows dynamic libraries don't have ``lib`` prefix and end with ``.dll``

The application and the device drivers can be obtained by following the build instructions of this repo. 
The command map is a part of the firmware code and built separately; see relevant documentation for the target for instructions on how to build and combine the command map.
To find out use cases and more information about the application use:

- on Linux and Mac

.. code-block:: console

    ./xvf_host --help

- on Windows

.. code-block:: console

    xvf_host.exe --help

The DFU host application is only supported on Raspbian, and it needs the following files in the same location:

- xvf_dfu
- libdevice_{protocol}.so
- dfu_cmds.yaml
- transport_config.yaml

To change the settings of the I2C and SPI transport protocols, edit the configurable values listed in *src/dfu/transport_config.yaml*.

*****************************************
Supported platforms and control protocols
*****************************************

- Raspberry Pi - arm7l (32-bit)
    - xvf_host
    - xvf_dfu
    - libdevice_i2c.so
    - libdevice_spi.so
    - libdevice_usb.so (only for xvf_host)
- Linux - x86_64
    - xvf_host
    - libdevice_usb.so
- Mac - x86_64
    - xvf_host
    - libdevice_usb.dylib
- Mac - arm64
    - xvf_host
    - libdevice_usb.dylib
- Windows - x86 (32-bit)
    - xvf_host.exe
    - device_usb.dll
