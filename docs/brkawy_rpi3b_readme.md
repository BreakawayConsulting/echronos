<!-- Copyright (c) 2019, Breakaway Consulting Pty. Ltd. -->

README
======

Overview
---------

This eChronos release provides support for the Raspberry Pi 3B series of single-board computers.
It should work on both the Raspberry Pi 3B and Raspberry Pi 3B+ models.
Testing of this release has been performed on the newer Raspberry Pi 3B+ model.

The RTOS variants available in this release do not support executing on multiple cores.
The RTOS will execute on core zero, while other cores will be held in reset.

However, this release does include rudimentary platform support for multiple core.
The hellomulti example system demonstrates the platform support for multiple cores.
This platform support forms the basis for future multi-core aware RTOS variants.

This release includes an initial release of the Lyrae RTOS variant.
The Lyrae variant supports the new *task group* concept which enables priority based preemption of groups of tasks.

Recommended Hardware
---------------------

The following is the recommended hardware for use with this release:

Raspberry Pi 3B+ (https://core-electronics.com.au/raspberry-pi-3-model-b-plus.html)
16GB Micro SD card (https://core-electronics.com.au/16gb-microsd-card-with-noobs-for-all-raspberry-pi-boards.html)
USB Micro 2.5A power supply (https://core-electronics.com.au/raspberry-pi-3-power-supply.html)
USB to TTL UART cable (https://core-electronics.com.au/usb-to-ttl-serial-cable.html)
Micro SD card reader/writer (https://core-electronics.com.au/usb-microsd-card-reader-writer-microsd-microsdhc-microsdxc.html)

It is recommended that a dedicated power supply is used rather than relying on power via a computer USB port.
It is possible that during development the board will draw a high current that is not handled by a USB port, which can lead to board resets which can be difficult to debug.

It is recommended that an SD card with an existing Raspberry Pi operating system is used.
This is not strictly necessary but makes configuration and setup easier.
Note: A 16GB SD card is recommended as compared to a 32GB SD card.
32GB SD cards will use the exFAT filesystem by default, which is not supported by the Raspberry Pi.

The USB to UART cable linked above is recommended as each of the sockets are clearly labelled allow simple, error-free connection to the Raspberry Pi.

Any SD card reader / writer can be used.
Many computers will have a full-size SD card reader / writer in which case a simple micro-SD to full-size-SD adapter is all that is required.


Hardware Setup
---------------

The hardware setup is relatively straight-forward.
The power supply should be connected to the micro-USB port on the Raspberry Pi.
It is recommended that the power supply is connected to a socket with a dedicated switch; during development you will be power-cycling the Raspberry Pi often, and this is best done with a power-switch rather than removing and inserting the USB cable.

The ground (GND), receive (RXD) and transmit (TXD) sockets on the USB to UART cable should be attached to pins 6, 8 and 10 on the Raspberry Pi.
See https://pinout.xyz/ for a Raspberry Pi pinout diagram.
Note: The TXD pin on the Raspberry Pi should be connected to the RXD socket on the USB to UART cable.
Similarly the RXD pin on the Raspberry Pi should be connected to the TXD socket on the USB to UART cable.

The micro SD card will be inserted in to the micro SD socket on the back of the Raspberry Pi.
Note: you will be moving the micro SD card between your reader/writer and the Raspberry Pi often; set up your hardware to make this easy.


Software Setup
---------------

This release is for a Windows host-computer.

You will need an appropriate Terminal emulator to view debug output.
Some options are TeraTerm (https://ttssh2.osdn.jp/) or putty (https://putty.org/).
These should be configured for communicating over the USB to UART cable with settings 115200 8N1.

You will need to install appropriate virtual COM port drivers for your USB to UART cable.
If using the recommended cable (or many others) then the FTDI drivers are required.
See this support page https://www.ftdichip.com/Drivers/VCP.htm to download the drivers.

Python 3.4 or later is required.

GCC ARM compiler toolchain is required.
The recommended toolchain for Windows can be downloaded from: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-a/downloads
The toolchain: gcc-arm-8.3-2019.03-i686-mingw32-aarch64-elf.tar.xz is required for this release.
Note: The xz compression format may required a 3rd party tool such as 7-Zip to extract.

Your will need to add the toolchain `bin` directory to your `PATH` environment variable.
After updating your `PATH` environment variable you should test by running the command `aarch64-elf-gcc` in a Powershell or command window.
If the toolchain is correctly installed this should fail with a message `no input files`.


Preparing the SD card
-----------------------

The default SD card comes formatted with a FAT filesystem and installed with a number of files.

The file `config.txt` controls how the bootloader on the Raspberry Pi works.
It must be changed to support booting the eChronos example systems rather than pre-configured Linux system.

The full contents of the file should be replaced with:

    gpu_mem=64
    start_file=start.elf
    fixup_file=fixup.dat
    boot_delay=0
    disable_splash=1
    enable_uart=1
    arm_64bit=1
    kernel=system

These options will configure the bootloader to enable 64-bit mode and the debug UART.
The splash screen will be disabled.
The `kernel` option specifies the filename of the firmware the be loaded.
For us this will be the `system` file, which is explained more in the following section.


Building the Hello World example
----------------------------------

This section describes how to build the *Hello World* example.
The approach described here can be applied to the other examples available in this release.

The following command will build the *Hello World* example:

    > .\bin\prj build machine-rpi3b.example.hello

This command runs the *prj* tool, passing the *build* sub-command.

The *machine-rpi3b.example.hello* specifies the system that should be built.

This example system is specified by the file `share\packages\machine-rpi3b\example\hello.prx`.

If the build succeeded then the output file shall be created as `out\machine-rpi3b\example\hello\system`

This system file should be copied to the SD card so that it can be run on the Raspberry Pi.

For example, if the SD card is mounted as the *E:* drive then the following command can be used:

    > copy out\machine-rpi3b\example\hello\system E:

Running a system
------------------

After building the system and copying it to the SD card it is time to run the system.
The SD card should be replaced in the Raspberry Pi and then the power supply should be turned on.
