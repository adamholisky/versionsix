.DEFAULT_GOAL := all

ROOT_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

DEFINES = -DVIOS -DAVSOS_KERNEL

CC = /usr/local/osdev/bin/x86_64-elf-gcc
ASM = /usr/local/osdev/bin/x86_64-elf-as
LD = /usr/local/osdev/bin/x86_64-elf-ld
OBJDUMP = /usr/local/osdev/bin/x86_64-elf-objdump

BUILD_LOG = $(ROOT_DIR)/build_temp/build.log
SHELL :=/bin/bash -O globstar
VPATH = $(shell find ./src -type d -printf "src/%P:")

SOURCES_C = $(shell ls src/**/*.c)
OBJECTS_C = $(patsubst %.c, build/%.o, $(shell ls src/**/*.c | xargs -n 1 basename))
EXECS_C = $(patsubst %.c, finished/%.exec, $(shell ls src/**/*.c | xargs -n 1 basename))

CFLAGS_REDUCED = $(DEFINES) -Wno-write-strings \
	-Wno-pointer-to-int-cast \
	-Wno-discarded-qualifiers \
	-Wno-int-conversion \
	-Wno-incompatible-pointer-types \
	-ffreestanding \
	-O0 \
	-g \
	-I$(ROOT_DIR)/../kernel/include \
	-I$(ROOT_DIR)/../avs-libc-static/include \
	-I$(ROOT_DIR)/../liblua-static/include \
	-I$(ROOT_DIR)/src \
    -m64                 \
    -march=x86-64        \
    -mabi=sysv           \
	-std=c11 \
	-lgcc \
	-nostdlib

CFLAGS_APP_SPECIFIC = -fPIC 

all: install 
#glue/crt0.o $(OBJECTS_C) $(EXECS_C)

finished/%.exec: build/%.o
	@$(eval OBJNAME := $(shell basename $@))
	@>&2 printf "[Link] $(OBJNAME)\n"
	$(LD) -shared -E -Bsymbolic -z max-page-size=0x1000 -o finished/$(OBJNAME) glue/crt0.o $<  >> $(BUILD_LOG)
	readelf -W -a finished/$(OBJNAME) > build_temp/elfdump-$(OBJNAME).txt
	$(OBJDUMP) -x -D -S finished/$(OBJNAME) > build_temp/objdump-$(OBJNAME).txt

build/%.o: %.c
	@$(eval OBJNAME := $(shell basename $@))
	@>&2 printf "[Compile] $(OBJNAME)\n"
	$(CC) $(CFLAGS_REDUCED) $(CFLAGS_APP_SPECIFIC) -c $< -o build/$(OBJNAME) >> $(BUILD_LOG)

glue/crt0.o: glue/crt0.S
	@>&2 printf "[Build] $<\n"
	$(CC) $(CFLAGS_REDUCED) $(CFLAGS_APP_SPECIFIC) -c glue/crt0.S -o glue/crt0.o >> $(BUILD_LOG)

apps: $(EXEC_C)

install:
	@make install_stage2 >> $(BUILD_LOG)
	@>&2 printf "[Install] Done\n"

install_stage2: glue/crt0.o $(EXECS_C)
	@make $(EXECS_C)
	cd /home/adam/code/afs-fuse && sudo ./asvfs-fuse --image=/usr/local/osdev/source/versionsix/asvfs_drive.img testmnt && sudo cp -f /usr/local/osdev/source/versionsix/core_apps/finished/* testmnt/bin && sudo fusermount -u testmnt

clean:
	rm -rf build/*.o
	rm -rf glue/*.o
	rm -rf build_temp/*
	rm -rf finished/*

debug_dump:
	@echo "Start Debug Dump"
	@echo "----------"
	@echo "vpath:" $(VPATH)
	@echo "----------"
	@echo "*.c:" $(SOURCES_C)
	@echo " "
	@echo "*.o:" $(OBJECTS_C)
	@echo " "
	@echo "*.exec:" $(EXECS_C)
	@echo "----------"
	@echo "End Debug Dump"
	@echo " "
