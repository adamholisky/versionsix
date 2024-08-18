.DEFAULT_GOAL := all

ROOT_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
DEFINES = -DVIFS_OS_ENV -DVI_ENV_OS

include $(ROOT_DIR)/build_support/control/paths.mk 
include $(ROOT_DIR)/build_support/control/toolchain.mk 

SOURCES_C = $(shell ls kernel/**/*.c)
SOURCES_ASMS = $(shell ls kernel/**/*.S)
OBJECTS_C = $(patsubst %.c, build/%.o, $(shell ls kernel/**/*.c | xargs -n 1 basename))
OBJECTS_ASMS = $(patsubst %.S, build/%.o, $(shell ls kernel/**/*.S | xargs -n 1 basename))

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
	$(QEMU) $(QEMU_COMMON) $(QEMU_DISPLAY_NORMAL) $(QEMU_DEBUG_COMMON) $(QEMU_DEBUG_LOGGING)

gdb:
	gdb -q --command=$(ROOT_DIR)/build_support/gdb_control/tui.gdb

gdbseer:
	seergdb --connect localhost:5894 /usr/local/osdev/versions/versionsix/build/versionvi.bin

gdbfrontend:
	gdbfrontend -G "--command=$(ROOT_DIR)/build_support/gdb_control/commands.gdb"

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
	@dd if=/dev/zero of=$(MOUNT_IMG) bs=100M count=2 >> $(BUILD_LOG)
	@$(eval LOOP_DRIVE := $(shell sudo losetup -f))
	@sudo losetup -fP $(MOUNT_IMG)
	@echo -e "g\nn\n1\n\n+100M\nn\n2\n\n\nw" | sudo fdisk $(LOOP_DRIVE) >> $(BUILD_LOG)
	@sudo mke2fs $(LOOP_DRIVE)p1
	@sudo mke2fs $(LOOP_DRIVE)p2
	@sudo mount $(LOOP_DRIVE)p1 $(MOUNT_DIR)
	@sudo cp -r build_support/boot_files/* $(MOUNT_DIR)
	@sudo umount $(MOUNT_DIR)
	@sudo /usr/local/osdev/share/limine/limine-deploy $(LOOP_DRIVE)
	@sudo losetup -d $(LOOP_DRIVE)

drive:
	@test $(ROOT_DIR)/afs.img || gzip $(ROOT_DIR)/afs.img
	@test $(ROOT_DIR)/scratch/backup.afs.img.gz || rm $(ROOT_DIR)/scratch/backup.afs.img.gz
	@test $(ROOT_DIR)/afs.img.gz || mv $(ROOT_DIR)/afs.img.gz $(ROOT_DIR)/scratch/backup.afs.img.gz
	@$(ROOT_DIR)/../vifs/vifs new 10 -afs $(ROOT_DIR)/afs.img >> $(BUILD_LOG)
	@$(ROOT_DIR)/../vifs/vifs bootstrap 0 -afs $(ROOT_DIR)/afs.img >> $(BUILD_LOG)
	@$(ROOT_DIR)/../vifs/vifs cpdir $(ROOT_DIR)/os_root / -afs $(ROOT_DIR)/afs.img >> $(BUILD_LOG)
	@>&2 echo [Make AFS Drive] Done

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