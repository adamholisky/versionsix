#The AVSOS Ecosystem

AVSOS is developed with a suite of tools that all interact with each other in one way or another. These include:

- *AVSOS*: A Very Simple OS. The kernel/operating system everything supports. 
- *AVSFS*: A Very Simple File System. Custom built file system for linux (through FUSE) and AVSOS.
- *AVSUL*: A Very Simple Utility Library. Things like list, trees, and bit arrays.
- *AVS Lib C*: A C-Library being developed alongside AVSOS. Large enough to allow some basic porting of third party software and libraries.
- *AVS Dev Desktop*: A browser based development desktop, showing VNC'd screen, multiple logs, debug outputs/terminals, and quick build options.
- *AVS Dev API*: Connecting up the kernel, apps running inside AVSOS, QEMU, the build system, and other tools together to deliver unified debug logging, system interaction, and basic automation.