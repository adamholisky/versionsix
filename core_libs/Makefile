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
SHARED_LIBS = $(patsubst %.c, build/%.so, $(shell ls src/**/*.c | xargs -n 1 basename))

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
	-I$(ROOT_DIR)/include \
    -m64                 \
    -march=x86-64        \
    -mabi=sysv           \
	-std=c11 \
	-lgcc \
	-nostdlib

CFLAGS_LIB_SPECIFIC = -fPIC 

all: install 

#	$(CC) -g -Wno-error -fPIC -c testlib.c -o testlib.o
#	$(CC) -g -fPIC -shared -o libtest.so testlib.o
#	objdump -x -D -S libtest.so > libtest-objdump.txt
#	readelf -W -a libtest.so > libtest-elfdump.txt

build/%.so: build/%.o
	@$(eval OBJNAME := $(shell basename $@))
	@>&2 printf "[Link] $(OBJNAME)\n"
	$(LD) -g -fPIC -shared -z max-page-size=0x1000 -o build/$(OBJNAME) $<  >> $(BUILD_LOG)
	readelf -W -a build/$(OBJNAME) > build_temp/elfdump-$(OBJNAME).txt
	$(OBJDUMP) -x -D -S build/$(OBJNAME) > build_temp/objdump-$(OBJNAME).txt

build/%.o: %.c
	@$(eval OBJNAME := $(shell basename $@))
	@>&2 printf "[Compile] $(OBJNAME)\n"
	$(CC) $(CFLAGS_REDUCED) $(CFLAGS_LIB_SPECIFIC) -c $< -o build/$(OBJNAME) >> $(BUILD_LOG)

libs: $(SOURCES_C)

install:
	@make install_stage2 >> $(BUILD_LOG)
	@>&2 printf "[Install] Done\n"

install_stage2: $(SHARED_LIBS)
	@make $(SHARED_LIBS)
	cd /home/adam/code/afs-fuse && sudo ./asvfs-fuse --image=/usr/local/osdev/source/versionsix/asvfs_drive.img testmnt && sudo cp -f /usr/local/osdev/source/versionsix/core_libs/build/*.so testmnt/lib && sudo fusermount -u testmnt

clean:
	rm -rf build/*.o
	rm -rf build/*.so
	rm -rf build_temp/*

debug_dump:
	@echo "Start Debug Dump"
	@echo "----------"
	@echo "vpath:" $(VPATH)
	@echo "----------"
	@echo "*.c:" $(SOURCES_C)
	@echo " "
	@echo "*.o:" $(OBJECTS_C)
	@echo " "
	@echo "*.exec:" $(SHARED_LIBS)
	@echo "----------"
	@echo "End Debug Dump"
	@echo " "
