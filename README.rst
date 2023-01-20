===========================
HOST_XVF_CONTROL Repository
===========================

HOST_XVF_CONTROL is host control reference application to be used for XVF3800 and onwards.

*******
Cloning
*******

Some dependent components are included as git sub modules. These can be obtained by cloning this repository with the following command:

.. code-block:: console

    git clone --recurse-submodules git@github.com:xmos/sw_xvf_host.git

********
Building
********

Build with cmake from the host_xvf_contol/ folder:

.. tab:: Linux and Mac

    .. code-block:: console

        cmake -B build && cd build && make

.. tab:: Windows

    .. code-block:: console

        # building with VS tools
        cmake -G "NMake Makefiles" -B build && cd build && nmake

*****
Using
*****

In order to use the application you should have the following files in the same location:

- xvf_host(.exe)
- (lib)command_map.(so/dll/dylib)
- (lib)device_{protocol}.(so/dll/dylib)

.. note::
    - Linux dynamic libraries end with ``.so``
    - Apple dynamic libraries end with ``.dylib``
    - Windows dynamic libraries don't have ``lib`` prefix and end with ``.dll``

The application and the device drivers can be obtained by following the build instructions of this repo. Command map is a part of the firmware code and built separately.
To find out use cases and more information about the application use:

.. tab:: Linux and Mac

    .. code-block:: console

        ./xvf_host --help

.. tab:: Windows

    .. code-block:: console

        xvf_host.exe --help

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

.. note:: 
    Mac and Windows builds don't have hardware drivers for now. If you want to test Mac or Windows applications,
    use ``-DTESTING=ON`` option when you do cmake, it will build dummy device and command_map dynamic libraries for all platforms.
