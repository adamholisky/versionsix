CC = /usr/local/osdev/bin/x86_64-elf-gcc
ASM = /usr/local/osdev/bin/x86_64-elf-as
LD = /usr/local/osdev/bin/x86_64-elf-ld
OBJDUMP = /usr/local/osdev/bin/x86_64-elf-objdump

CFLAGS = $(DEFINES) -Wno-write-strings \
	-Wno-pointer-to-int-cast \
	-Wno-discarded-qualifiers \
	-Wno-int-conversion \
	-Wno-incompatible-pointer-types \
	-ffreestanding \
	-fno-omit-frame-pointer \
	-fno-lto             \
	-fno-stack-protector \
    -fno-stack-check     \
	-mno-red-zone        \
	-O0 \
	-g \
	-I$(ROOT_DIR)/kernel/include \
	-I$(ROOT_DIR)/vi_klibc_static/include \
    -m64                 \
    -march=x86-64        \
    -mabi=sysv           \
    -mcmodel=kernel      

CFLAGS_END = -nostdlib -lgcc
AFLAGS = $(CFLAGS)

QEMU = /usr/bin/qemu-system-x86_64
QEMU_COMMON = 	-readconfig ${ROOT_DIR}/build_support/qemu_configs/x86_64_primary.cfg \
				\
				-d cpu_reset \
				\
				-no-reboot \
				\
				-pidfile ${ROOT_DIR}/build_support/logs/qemu_pid
QEMU_DISPLAY_NONE =	-display none
QEMU_DISPLAY_CURSES = -display curses
QEMU_DISPLAY_NORMAL = -vga std -display gtk,gl=on
QEMU_DISPLAY_VNC = -vnc 0.0.0.0:52102,websocket=58003
QEMU_DEBUG_COMMON = -S -gdb tcp::58001 
QEMU_DEBUG_LOGGING = -D $(ROOT_DIR)/build_support/logs/qemu_debug_log.txt 
