.DEFAULT_GOAL := all

# ROOT_DIR needs to be defined in the primary Makefile up top
ROOT_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

# Makefile build supporter functions/vars
include $(ROOT_DIR)/build_support/make_files/build_helpers.mk

# Path import
include $(ROOT_DIR)/build_support/make_files/paths.mk

# Build number 
# Incrementing lives under rule "increment_build_number"
ifeq ( $(MAKECMDGOALS), 'build/versionvi.bin' )
	$(shell make increment_build_number)
endif

$(eval BUILD_NUMBER = $(shell echo $$(($$(cat $(BUILD_NUMBER_FILE)) + 1))))

# Defines
DEFINES = -DVIFS_OS_ENV 
DEFINES += -DSTDIO_SERIAL_3
#DEFINES += -DENABLE_GUI
DEFINES += -DKLOG_AVS_DEV_API_OUT
DEFINES += -DVI_ENV_OS 
DEFINES += -DVIOS 
DEFINES += -DBUILD_NUM=$(BUILD_NUMBER) 


# Toolchain
CC = /usr/local/osdev/bin/x86_64-elf-gcc
ASM = /usr/local/osdev/bin/x86_64-elf-as
LD = /usr/local/osdev/bin/x86_64-elf-ld
OBJDUMP = /usr/local/osdev/bin/x86_64-elf-objdump

PORT_GDB = 58001

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
				-m 8G \
				\
				-d cpu_reset \
				\
				-no-reboot \
				\
				-pidfile ${ROOT_DIR}/logs/qemu_pid
QEMU_DISPLAY_NONE =	-display none
QEMU_DISPLAY_CURSES = -display curses
QEMU_DISPLAY_NORMAL = -vga std -no-shutdown
QEMU_DISPLAY_VNC = -vnc 0.0.0.0:52102,websocket=58003
QEMU_DEBUG_COMMON = -S -gdb tcp::$(PORT_GDB)
QEMU_DEBUG_LOGGING = -d cpu_reset -D $(ROOT_DIR)/logs/qemu_debug_log.txt


SOURCES_C = $(shell ls kernel/**/*.c)
SOURCES_ASMS = $(shell ls kernel/**/*.S)
OBJECTS_C = $(patsubst %.c, build/%.o, $(shell ls kernel/**/*.c | xargs -n 1 basename))
OBJECTS_ASMS = $(patsubst %.S, build/%.o, $(shell ls kernel/**/*.S | xargs -n 1 basename))

all: install

build/versionvi.bin: increment_build_number $(OBJECTS_C) $(OBJECTS_ASMS)
	$(LD) -nostdlib -static -m elf_x86_64 -z max-page-size=0x1000 -T build_support/linker.ld -o build/versionvi.bin vi_klibc_static/vklibc.o $(OBJECTS_C) $(OBJECTS_ASMS)
	readelf -W -a build/versionvi.bin > logs/elfdump.txt
	@>&2 $(call echo_tag_green,Build, Done making version $(BUILD_NUMBER))

build/%.o: %.c
	@>&2 $(call echo_tag_green,Build, $<)
	$(eval OBJNAME := $(shell basename $@))
	$(CC) $(CFLAGS) $(CFLAGS_END) -std=c11 -c $< -o build/$(OBJNAME) >> $(BUILD_LOG)

build/%.o: %.S
	@>&2 $(call echo_tag_green,Build, $<)
	$(eval OBJNAME := $(shell basename $@))
	$(CC) $(AFLAGS) -c $< -o build/$(OBJNAME) >> $(BUILD_LOG)

dumpobjs:
	$(OBJDUMP) -x -D -S build/versionvi.bin > logs/objdump.txt 

cp: cp_fs cp_vit

cp_fs:
	@cp -f ../vifs/src/vfs.c kernel/fs/vfs.c
	@cp -f ../vifs/src/rfs.c kernel/fs/rfs.c
	@cp -f ../vifs/src/afs.c kernel/fs/afs.c
	@cp -f ../vifs/include/vfs.h kernel/include/vfs.h
	@cp -f ../vifs/include/rfs.h kernel/include/rfs.h
	@cp -f ../vifs/include/afs.h kernel/include/afs.h

cp_vit:
	@cp -f ../viui/include/vit.h kernel/include/vit.h

	@cp -f ../viui/include/lib/bitmap.h kernel/include/lib/bitmap.h
	@cp -f ../viui/src/lib/bitmap.c kernel/lib/bitmap.c

	@cp -f ../viui/include/lib/hash.h kernel/include/lib/hash.h
	@cp -f ../viui/src/lib/hash.c kernel/lib/hash.c

	@cp -f ../viui/include/lib/dictionary.h kernel/include/lib/dictionary.h
	@cp -f ../viui/src/lib/dictionary.c kernel/lib/dictionary.c

	@cp -f ../viui/include/vui/button.h kernel/include/vui/button.h
	@cp -f ../viui/include/vui/console.h kernel/include/vui/console.h
	@cp -f ../viui/include/vui/desktop.h kernel/include/vui/desktop.h
	@cp -f ../viui/include/vui/event.h kernel/include/vui/event.h
	@cp -f ../viui/include/vui/font.h kernel/include/vui/font.h
	@cp -f ../viui/include/vui/label.h kernel/include/vui/label.h
	@cp -f ../viui/include/vui/layout.h kernel/include/vui/layout.h
	@cp -f ../viui/include/vui/menu.h kernel/include/vui/menu.h
	@cp -f ../viui/include/vui/menubar.h kernel/include/vui/menubar.h
	@cp -f ../viui/include/vui/vui.h kernel/include/vui/vui.h
	@cp -f ../viui/include/vui/window.h kernel/include/vui/window.h
	@cp -f ../viui/include/vui/schrift.h kernel/include/vui/schrift.h

	@cp -f ../viui/src/vui/button.c kernel/vui/button.c
	@cp -f ../viui/src/vui/console.c kernel/vui/console.c
	@cp -f ../viui/src/vui/desktop.c kernel/vui/desktop.c
	@cp -f ../viui/src/vui/draw.c kernel/vui/draw.c
	@cp -f ../viui/src/vui/event.c kernel/vui/event.c
	@cp -f ../viui/src/vui/font.c kernel/vui/font.c
	@cp -f ../viui/src/vui/label.c kernel/vui/label.c
	@cp -f ../viui/src/vui/layout.c kernel/vui/layout.c
	@cp -f ../viui/src/vui/menu.c kernel/vui/menu.c
	@cp -f ../viui/src/vui/menubar.c kernel/vui/menubar.c
	@cp -f ../viui/src/vui/vui.c kernel/vui/vui.c
	@cp -f ../viui/src/vui/window.c kernel/vui/window.c
	@cp -f ../viui/src/vui/schrift.c kernel/vui/schrift.c

install:
	@make install_stage2 >> $(BUILD_LOG)
	@>&2 $(call echo_tag_green,Install, Build $(BUILD_NUMBER) is now installed)
	@>&2 $(call echo_tag_green,Install, Done)

install_stage2: build/versionvi.bin
	@>&2 $(call echo_tag_green,Install, Installing to $(KERNEL_BOOT_IMG))
	@mcopy -D o -i $(ROOT_DIR)/$(KERNEL_BOOT_IMG)@@1M $(ROOT_DIR)/build_support/boot_files/limine.conf ::/boot/limine
	@mcopy -D o -i $(ROOT_DIR)/$(KERNEL_BOOT_IMG)@@1M $(ROOT_DIR)/build/versionvi.bin ::/boot

#& /mnt/c/"Program Files"/TightVNC/tvnviewer.exe :0
run: install
	$(QEMU) $(QEMU_COMMON) $(QEMU_DISPLAY_NORMAL) $(QEMU_DEBUG_LOGGING)

run-term:  install
	$(QEMU) $(QEMU_COMMON) $(QEMU_DISPLAY_NONE) $(QEMU_DEBUG_LOGGING)


run-vnc:  install
	$(QEMU) $(QEMU_COMMON) $(QEMU_DISPLAY_VNC) $(QEMU_DEBUG_LOGGING)

run-no-install:
	$(QEMU) $(QEMU_COMMON) $(QEMU_DISPLAY_NORMAL) $(QEMU_DEBUG_LOGGING) 

run-term-no-install:
	$(QEMU) $(QEMU_COMMON) $(QEMU_DISPLAY_CURSES) $(QEMU_DEBUG_LOGGING) 

run-debug: install
	$(QEMU) $(QEMU_COMMON) $(QEMU_DISPLAY_NORMAL) $(QEMU_DEBUG_COMMON) $(QEMU_DEBUG_LOGGING)

run-debug-term: install
	$(QEMU) $(QEMU_COMMON) $(QEMU_DISPLAY_NONE) $(QEMU_DEBUG_COMMON) $(QEMU_DEBUG_LOGGING)

gdb:
	gdb -q --command=$(ROOT_DIR)/build_support/gdb_control/tui.gdb

gdbseer:
	seergdb --connect localhost:$(PORT_GDB) $(ROOT_DIR)/build/versionvi.bin

gdbfrontend:
	/home/adam/code/gdb-frontend/gdbfrontend --host=plato.marsdev.io --listen=plato.marsdev.io --port=58022 --dontopenuionstartup -G "--command=$(ROOT_DIR)/build_support/gdb_control/commands.gdb"

debug_dump:
	@>&2 echo [Build] Makefile Debug Dump
	@make debug_dump_stage2 >> $(BUILD_LOG)

debug_dump_stage2:
	@echo "Start Debug Dump"
	@echo "----------"
	@echo "vpath:" $(VPATH)
	@echo "----------"
	@echo "*.s:" $(SOURCES_ASM)
	@echo " "
	@echo "*.o:" $(OBJECTS_ASM)
	@echo "----------"
	@echo "*.S:" $(SOURCES_ASMS)
	@echo " "
	@echo "*.o:" $(OBJECTS_ASMS)
	@echo "----------"
	@echo "*.c:" $(SOURCES_C)
	@echo " "
	@echo "*.o:" $(OBJECTS_C)
	@echo "----------"
	@echo "End Debug Dump"
	@echo " "

create_img:
	@make create_img_stage_2 >> $(BUILD_LOG)
	@>&2 echo [Create Img] Done

#	This is failing on arch linux? Gives noacl error. Removing.
# 	@sudo mount -o noacl $(LOOP_DRIVE)p1 $(MOUNT_DIR)
create_img_stage_2:
	dd if=/dev/zero of=$(MOUNT_IMG) bs=200M count=2 >> $(BUILD_LOG)
	$(eval LOOP_DRIVE := $(shell sudo losetup -f))
	sudo losetup -fP $(MOUNT_IMG)
	echo -e "g\nn\n1\n\n+100M\nn\n2\n\n\nw" | sudo fdisk $(LOOP_DRIVE) >> $(BUILD_LOG)
	sudo mke2fs $(LOOP_DRIVE)p1
	sudo mke2fs $(LOOP_DRIVE)p2
	sudo mount $(LOOP_DRIVE)p1 $(MOUNT_DIR)
	sudo cp -r build_support/boot_files/* $(MOUNT_DIR)
	sudo umount $(MOUNT_DIR)
	sudo echo "Running limine install...\n"
	sudo /usr/local/osdev/bin/limine bios-install $(LOOP_DRIVE)
	sudo losetup -d $(LOOP_DRIVE)

drive:
	@test $(ROOT_DIR)/afs.img || gzip $(ROOT_DIR)/afs.img
	@test $(ROOT_DIR)/scratch/backup.afs.img.gz || rm $(ROOT_DIR)/scratch/backup.afs.img.gz
	@test $(ROOT_DIR)/afs.img.gz || mv $(ROOT_DIR)/afs.img.gz $(ROOT_DIR)/scratch/backup.afs.img.gz
	@$(ROOT_DIR)/../vifs/vifs new 50 -afs $(ROOT_DIR)/afs.img >> $(BUILD_LOG)
	@$(ROOT_DIR)/../vifs/vifs bootstrap 0 -afs $(ROOT_DIR)/afs.img >> $(BUILD_LOG)
	@$(ROOT_DIR)/../vifs/vifs cpdir $(ROOT_DIR)/os_root / -afs $(ROOT_DIR)/afs.img >> $(BUILD_LOG)
	@>&2 echo [Make AFS Drive] Done

.PHONY: increment_build_number
increment_build_number:
	$(eval $(shell echo $(BUILD_NUMBER) > $(BUILD_NUMBER_FILE)))

.PHONY: clean
clean:
	@$(call echo_tag_yellow,Clean, Starting cleanup)
	@rm -rf build_support/logs/build.log
	@make clean_stage_2 >> $(BUILD_LOG)
	@$(call echo_tag_yellow,Clean, Done)

.PHONY: clean_stage_2
clean_stage_2:
	rm -rf build/*.o 
	rm -rf build/*.bin 
	rm -rf logs/objdump.txt 
	rm -rf logs/elfdump.txt
	rm -rf logs/qemu_debug_log.txt