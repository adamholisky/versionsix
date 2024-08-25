CC = /usr/local/osdev/bin/x86_64-elf-gcc
ASM = /usr/local/osdev/bin/x86_64-elf-as
LD = /usr/local/osdev/bin/x86_64-elf-ld
OBJDUMP = /usr/local/osdev/bin/x86_64-elf-objdump

#CFLAGS = $(DEFINES) -Wno-write-strings -fcompare-debug-second -ffreestanding -fno-omit-frame-pointer -O0 -g -I$(ROOT_DIR)/kernel/include -I$(ROOT_DIR)/../libcvv/libc/include

#-finstrument-functions \
	-finstrument-functions-exclude-file-list=helper_asm.S,interrupt_asm.S,debug.c,serial.c,ksymbols.c,bootstrap.c,interrupt.c,write.c \
	-finstrument-functions-exclude-function-list=inportb,outportb,in_port_long,out_port_long,timer_handler

CFLAGS = $(DEFINES) -Wno-write-strings \
	-Wno-pointer-to-int-cast \
	-Wno-discarded-qualifiers \
	-ffreestanding \
	-fno-omit-frame-pointer \
	-fno-lto             \
	-fno-stack-protector \
    -fno-stack-check     \
	-mno-red-zone        \
	-O0 \
	-g \
	-I$(ROOT_DIR)/kernel/include \
	-I$(ROOT_DIR)/../libcvv/libc/include \
    -m64                 \
	-mno-sse \
    -march=x86-64        \
    -mabi=sysv           \
    -mcmodel=kernel      

CFLAGS_END = -nostdlib -lgcc
AFLAGS = $(CFLAGS)

# -serial file:$(ROOT_DIR)/build_support/logs/serial_out.txt 
# -serial telnet:127.0.0.1:99,server=on,wait=off 
# pci_cfg_read pci_cfg_write
# -d trace:"e1000*",trace:"pic_interrupt"

# -nic user,model=e1000,ipv6=off,ipv4=on,mac=12:34:56:78:9A:BC \

# -netdev user,id=private_net,ipv6=off,ipv4=on \
-device e1000,netdev=private_net,mac=12:34:56:78:9A:BC \
-object filter-dump,id=f1,netdev=private_net,file=$(ROOT_DIR)/build_support/logs/packets.dat \

# -netdev socket,id=privatenet,listen=:1234
# 
# -netdev tap,ifname=tap0,br=br0,script=no,id=private_net \
				-device e1000,netdev=private_net,mac=12:34:56:78:9A:BC \
				-object filter-dump,id=f1,netdev=private_net,file=dump.dat 

# 				--enable-kvm \

QEMU = /usr/bin/qemu-system-x86_64
QEMU_COMMON = 	-device ahci,id=ahci \
				\
				--enable-kvm \
				\
				-drive id=main_drive,format=raw,if=none,file=$(ROOT_DIR)/vi_hd.img \
				-device ide-hd,drive=main_drive,bus=ahci.0 \
				\
				-drive id=secondary_drive,format=raw,if=none,file=$(ROOT_DIR)/afs.img \
				-device ide-hd,drive=secondary_drive,bus=ahci.1 \
				\
				-device isa-debug-exit,iobase=0xf4,iosize=0x04 \
				\
				-netdev user,id=private_net,ipv6=off,ipv4=on,restrict=off \
				-device e1000,netdev=private_net,mac=12:34:56:78:9A:BC \
				\
				-object filter-dump,id=f1,netdev=private_net,file=$(ROOT_DIR)/build_support/logs/packets.dat \
				\
				-m 8G \
				\
				-serial stdio \
				-serial null \
				-serial null \
				-serial file:$(ROOT_DIR)/build_support/logs/serial_out.txt \
				\
				-d cpu_reset \
				\
				-no-reboot
QEMU_DISPLAY_NONE =	-display none

#  -vnc :1
QEMU_DISPLAY_NORMAL = -vga std -display gtk,gl=on
QEMU_DEBUG_COMMON = -S -gdb tcp::5894 
QEMU_DEBUG_LOGGING = -D $(ROOT_DIR)/build_support/logs/qemu_debug_log.txt 