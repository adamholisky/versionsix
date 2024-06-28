# Version 6

Version 6 is a hobbist operating system designed to let one mess with underlying tech that normally isn't used in day-to-day high level development. I've been playing with OS development since college, and this is the sixth iteration (over 20-years) of my hobbyist OS.

All code is open sourced, written by Adam Holisky in his spare time unless otherwise noted.

Contact me VIA: Email adam.holisky@gmail.com -- Twitter [@adamholisky](https://x.com/AdamHolisky) -- BlueSky [@adamholisky.bsky.social](https://bsky.app/profile/adamholisky.bsky.social)

## What is Version 6

### Vision

A small, lean operating system/supervisor that allows direct hardware interfacing useful for tinkering with different aspects of modern technology. Runs on x86 and ARM processors with an abstracted technology layer, masking differences between architectures. 

### Current Goals

The "Big Goal" for right now is to laser focus on getting a Telnet connection established via TCP/IP over an emulated Ethernet device and network. 

| Goal | Status | Comments |
| ---- | ---- | ---- |
| Keep it simple | 🟢 | Prefer readablity over optimization whenever possible |
| C++, C, Assembly | 🟢 | Prefer C++ when possible, otherwise C and Assembly as needed |
| Clean build system | 🟢 | Don't over-complicated the build system, but don't shy away from advanced topics |
| Completely in x86_64, no 32bit code  | 🟢 | Loaded via [Limine](https://github.com/limine-bootloader/limine/blob/v7.x/README.md) |
| Basic paging | 🟡 | Allocating and page maping with own page tables |
| Basic memory management | 🟡 | Linear allocator |
| Basic interrupts | 🟡 | Enough to handle exceptions, timer, etc... |
| I/O via Serial | 🟢 | Debug out and stdio via QEMU's serial ports works |
| In-OS sanity checks | 🟡 | Some sanity checks run regardless, more run when debugging is enabled for specific code blocks |
| Basic console on framebuffer  | 🟡 | Good enough, maybe | 
| PCI | 🟡 | Good enough to get basic info for devices |
| Ethernet | 🟡 | Good enough, maybe |
| ARP | 🟡 | Protocol works, need to do dictionary |
| IP | 🟡 | Good enough, maybe |
| DHCP | 🟡 | Good enough, maybe |
| TCP | 🔴 | Not started |
| Telnet | 🔴 | Possible to hack together now via serial i/o and QEMU pipes |
| HTTP/Json | 🔴 | Not started |

### Future Goals

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

`make` run by itself in the root directory will build the system.

### Running

From the root directory, do a `make run` to execute the OS's normal operation path. 

For normal execution, the Makefile and QEMU are configured to send data accordingly:
- `build_support/logs/build.log` Output for the build commands 
- `build_support/logs/elfdump.txt` ELF information for compiled kernel
- `build_support/logs/objdump.txt` Full disassembly for compiled kernel, including final addresses via the linker
- `build_support/logs/serial_out.txt` Running serial log from OS's COM4 port output 
- `build_support/logs/qemu_debug_log.txt` has a bunch of detailed output from QEMU, very useful for tripple faults
- `stderr` Error output from G++, etc...
- `stdin/stdout` QEMU's "terminal" in/out, sending to/from in OS's COM1 port I/O

### Debugger 

From the root directory, run `make run_debug` to boot the OS with QEMU accepting remote connections to port 5984. Run `make gdb` to run GDB using the configuration in the `build_support/gdb_control/commands.gdb` file (TUI, some shortcuts, etc...)

Breakpoints are manually listed in the `commands.gdb` file.

## Kanban Task List
This is genereally the order that I'm going to build things.

1. ~~Limine boot into x86 64bit~~
1. ~~Makefile cleanup, logging~~
1. ~~Serial out~~
1. ~~Port the libc back to 64bit and verify it basically works~~
1. ~~Debug logging~~
1. ~~Find paging data from limine, use it or my own entirely~~
1. ~~Interrupts~~
1. ~~Interrupt debug display~~
1. ~~Page allocation and allocator~~
1. ~~Linear memory allocator~~
1. ~~Kernel symbols loaded via ELF processing, used in crash handling~~
1. Stackframe back trace
1. PCI
1. Ethernet
1. ARP
1. Ping
1. Telnet