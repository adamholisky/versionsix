# A Very Simple Operating System

<img width="1021" height="321" alt="Screenshot_20260526_194156" src="https://github.com/user-attachments/assets/251f9f16-abb5-4a4e-9d6e-3556756f5625" />

A Very Simple Operating System is designed to be ... simple. It's a kernel and user-space that's engineered to let one mess with underlying tech that normally isn't used in day-to-day high level development. There's a fair bit implemented in AVSOS, depending on how you look at it. But none of it's really "done" in any sense of the word.

All code is open sourced, written by Adam Holisky in his spare time unless otherwise noted.

Contact me VIA: Email adam.holisky@gmail.com -- BlueSky [@adam.holisky.com](https://bsky.app/profile/adam.holisky.com)

## Project Details

### Vision

A small, lean operating system/supervisor that allows direct hardware interfacing useful for tinkering with different aspects of modern technology. Runs on x86 with an abstracted technology layer, masking differences between architectures. 

### Project Goals

The "Big Goal" for right now is to laser focus on getting a Telnet connection established via TCP/IP over an emulated Ethernet device and network. 

| Goal | Status | Comments |
| ---- | ---- | ---- |
| Keep it simple | 🟢 | Prefer readablity over optimization whenever possible |
| C++, C, Assembly | 🟢 | Core OS in C. Assembly as needed. |
| Clean build system | 🟢 | Don't over-complicated the build system, but don't shy away from advanced topics |
| Completely in x86_64, no 32bit code  | 🟢 | Loaded via [Limine](https://github.com/limine-bootloader/limine/blob/v7.x/README.md) |
| Tool integration | 🟢 | Where appropriate use third-party tools and code to make things interesting |

## Technical Details

### Subsystem Support

Version 6 is made up of a lot of different systems working together. They're all in different states of maturity.

| Core System | Status | |
| ---- | ---- | ---- |
| Console | 🟢 | GUI and serial | 
| Paging | 🟢 | Allocating and page maping with own page tables, it works more or less |
| Memory Management | 🟢 | Implemented third party allocator, functions as expected |
| Processes | 🟢 | Init, relocation, elf loader, scheduler (yield), wait queues, fork, execve |
| File System | 🟢 | VFS, home-brewed custom file system, works with linux/fuse, read/write/etc work as expected |
| Interrupts | 🟢 | Handles most cases. Context manipulation mature enough to allow devices, debugging, and processes. |
| Devices | 🟢 | /dev file system, r/w on devices, virtual and real devices, keyboard, timers, redirection, etc... |
| I/O via Serial | 🟢 | Multiple I/O channels configured to work with QEMU. |
| Syscalls | 🟢 | Basics are working, expandable and quick |
| GUI | 🟢 | Modular, direct FB, working with TTF fonts, console, basic things | 
| In-OS sanity checks | 🟢 | Panics and attention-grabbing errors present thorughout |
| PCI | 🟢 | Good enough to get basic info for devices |
| libc | 🟢 | Good enough, compiled on its own |
| Threads | 🟢 | Good enough for basic thread support. Fragile. |
| Lua | 🟢 | Lua 5.5 integrated and working |
| User space | 🟡 | All basics except processor ring 3 working |
| klib | 🟡 | Starting to bring in other library routines for better data structure and routine support |
| Ethernet | 🟡 | Good enough, maybe |
| ARP | 🟡 | Protocol works, need to do dictionary |
| IP | 🟡 | Good enough, maybe |
| DHCP | 🟡 | Good enough, maybe |
| TCP | 🟡 | Working in a PoC stage |
| Telnet | 🟠 | Working in a very basic way |

### Directory Info

There's lots of files here. This is how they're all laid out.

| Directory | Use |
| ---- | ---- |
| `/build` | All compiled objects/output goes in here. |
| `/build/boot_files` | Limine config, to be copied into boot drives | 
| `/build/gdb_control` | GDB commands and configuration files |
| `/build/img_mount_point` | Local point for mounting virtual drive images | 
| `/make_files` | Makefile support files |
| `/qemu_configs` | QEMU system configuration files |
| `/core_apps` | Externally compiled apps to be put on FS and used in OS |
| `/kernel` | Source directory for the code, inside is self explanatory |
| `/logs` | All logs go here. There can be a lot. |
| `/os_root` | Root directory structure for the OS data drive |
| `/vi_klibc_static` | Home-brewed libc object file(s) and headers |
| `/scratch` | I don't believe in deleting anything, so stuff goes here | 

### Port List

QEMU and the overall dev enivornment exposes (and needs exposed) certain ports to function.
```
58000 - VIOS C&C Dashboard
58001 - QEMU GDB
58002 - QEMU VNC
58003 - Internal GDB Server serial output
58004 - QEMU Monitor over telnet
58005 - QEMU QCP over telnet
58006 - AVS Dev API Websocket
58007 - QEMU/COM3 -> Dev API
58020 - VS Code Web
58021 - Webssh2 websocket tunnel
58022 - GDB Frontend Web
```

### Future Tech Goals

| Goal | Status | Comments |
| ---- | ---- | ---- |
| ARM support | 🔴 | TBD |
| Smooth looking Text UI | 🔴 | Borland C called and wants its UI back |
| Scripting language | 🔴 | Sketched out basic HyperTalk-like language |
| Heap memory managment | 🔴 | Ready to be implemented, mostly independent of paging |
| Unit tests | 🔴 | TBD |
| Run on real hardware | 🔴 | Should be possible to do with a basic board |

## Compile and Usage Guide

### Host and Build Setup

The host system should have set up:
- Cross-compile GCC, G++, GAS, ELF Linker
- QEMU emulating x86_64
- Some sort of loopback/disk image mounting

Makefile targets are clearly outlined in the `Makefile`, with supported includes in the `build_support/control/*.mk` files, notably the `toolchain.mk` file which contains direct links to the compilers that the build system will use.

### Compling 

`make` run by itself in the root directory will build the system. Hard coded paths are in the root Makefile.

### Running

From the root directory, do a `make run` to execute the OS's normal operation path. 

For normal execution, the Makefile and QEMU are configured to send data accordingly:
- `logs/build.log` Output for the build commands 
- `logs/elfdump.txt` ELF information for compiled kernel
- `logs/objdump.txt` Full disassembly for compiled kernel, including final addresses via the linker
- `ogs/serial_out.txt` Running serial log from OS's COM4 port output 
- `logs/qemu_debug_log.txt` has a bunch of detailed output from QEMU, very useful for tripple faults
- `stdin/stdout` QEMU's "terminal" in/out, sending to/from in OS's COM3e port I/O

### Debugging 

From the root directory, run `make run-debug` to boot the OS with QEMU accepting remote connections to port 58001. Run `make gdb` to run GDB using the basic configuration in the `build_support/gdb_control/commands.gdb` file. TUI support included in `tui.gdb`, and breakpoints added in `breakpoints.gdb`.

## AI Statement

Generative AI is not used in this project at all. Claude is used to search documentation and code, and to help me remember what some obscure makefile syntax is that I haven't touched in two years. Every bit of code, however minor, has been written by human hands, and human hands alone.
