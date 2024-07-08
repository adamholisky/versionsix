.DEFAULT_GOAL := all

ROOT_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
DEFINES = -DPAGING_PAE -DGRAPHICS_OFF -DBITS_64 
MOUNT_IMG = vi_hd.img

include $(ROOT_DIR)/build_support/control/paths.mk 
include $(ROOT_DIR)/build_support/control/toolchain.mk 

SOURCES_C = $(shell ls **/*.c)
SOURCES_ASMS = $(shell ls **/*.S)
OBJECTS_C = $(patsubst %.c, build/%.o, $(shell ls **/*.c | xargs -n 1 basename))
OBJECTS_ASMS = $(patsubst %.S, build/%.o, $(shell ls **/*.S | xargs -n 1 basename))

all: debug_dump install

#$(CC) -T build_support/linker.ld -o build/versionvi.bin $(CFLAGS) ../libcvv/libc/vvlibc.o $(OBJECTS_C) $(OBJECTS_ASMS) $(CFLAGS_END)

build/versionvi.bin: $(OBJECTS_C) $(OBJECTS_ASMS)
	$(LD) -nostdlib -static -m elf_x86_64 -z max-page-size=0x1000 -T build_support/control/linker.ld -o build/versionvi.bin build_support/klibc/vvlibc.o $(OBJECTS_C) $(OBJECTS_ASMS)
	$(OBJDUMP) -x -D -S build/versionvi.bin > build_support/logs/objdump.txt
	readelf -W -a build/versionvi.bin > build_support/logs/elfdump.txt
	@>&2 printf "[Build] Done\n"

build/%.o: %.c
	@>&2 printf "[Build] $<\n"
	$(eval OBJNAME := $(shell basename $@))
	$(CC) $(CFLAGS) $(CFLAGS_END) -std=c11 -c $< -o build/$(OBJNAME) >> $(BUILD_LOG)

build/%.o: %.S
	@>&2 printf "[Build] $<\n"
	$(eval OBJNAME := $(shell basename $@))
	$(CC) $(AFLAGS) -c $< -o build/$(OBJNAME) >> $(BUILD_LOG)

install:
	@make install_stage2 >> $(BUILD_LOG)
	@>&2 printf "[Install] Done\n"

install_stage2: build/versionvi.bin
	@>&2 echo [Install] Installing to $(MOUNT_IMG)
	@$(eval LOOP_DRIVE := $(shell sudo losetup -f))
	@sudo losetup -fP $(MOUNT_IMG)
	@sudo mount $(LOOP_DRIVE)p1 $(MOUNT_DIR)
	@sudo cp build/versionvi.bin -f $(MOUNT_DIR)/versionvi.bin
	@sudo cp build_support/boot_files/limine.cfg -f $(MOUNT_DIR)/limine.cfg
	@sudo umount $(MOUNT_DIR) 
	@sudo losetup -d $(LOOP_DRIVE)

#& /mnt/c/"Program Files"/TightVNC/tvnviewer.exe :0
run: install
	$(QEMU) $(QEMU_COMMON) $(QEMU_DISPLAY_NORMAL) $(QEMU_DEBUG_LOGGING) 

run_debug: install
	$(QEMU) $(QEMU_COMMON) $(QEMU_DISPLAY_NORMAL) $(QEMU_DEBUG_COMMON)

gdb:
	gdbtui -q --command=$(ROOT_DIR)/build_support/gdb_control/commands.gdb

gdbseer:
	seergdb --connect localhost:5894 /usr/local/osdev/versions/vi/build/versionvi.bin

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

create_img_stage_2:
	@dd if=/dev/zero of=$(MOUNT_IMG) bs=100M count=2 >> $(BUILD_LOG)
	@$(eval LOOP_DRIVE := $(shell sudo losetup -f))
	@sudo losetup -fP $(MOUNT_IMG)
	@echo -e "g\nn\n1\n\n+100M\nn\n2\n\n\nw" | sudo fdisk $(LOOP_DRIVE) >> $(BUILD_LOG)
	@sudo mke2fs $(LOOP_DRIVE)p1
	@sudo mke2fs $(LOOP_DRIVE)p2
	@sudo mount -o noacl $(LOOP_DRIVE)p1 $(MOUNT_DIR)
	@sudo cp -r build_support/boot_files/* $(MOUNT_DIR)
	@sudo umount $(MOUNT_DIR)
	@sudo /usr/local/osdev/share/limine/limine-deploy $(LOOP_DRIVE)
	@sudo losetup -d $(LOOP_DRIVE)

clean:
	@rm -rf build_support/logs/build.log
	@make clean_stage_2 >> $(BUILD_LOG)

clean_stage_2:
	rm -rf build/*.o 
	rm -rf build/*.bin 
	rm -rf build_support/logs/objdump.txt 
	rm -rf build_support/logs/elfdump.txt
	rm -rf build_support/logs/qemu_debug_log.txt
	@>&2 echo [Clean] Done