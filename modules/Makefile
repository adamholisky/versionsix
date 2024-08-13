.DEFAULT_GOAL := all

SHELL :=/bin/bash -O globstar
VPATH = $(shell find ./src -type d -printf "src/%P:")
SOURCES := $(shell ls src/**/*.c)
OBJECTS := $(patsubst %.c, build/%.o, $(shell ls src/**/*.c | xargs -n 1 basename))

#Support variables
ROOT_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BUILD_LOG = $(ROOT_DIR)/build.log

#Compile programs and flags
CFLAGS = -ffreestanding -fno-omit-frame-pointer -O2 -nostdlib -static-libgcc -lgcc -g -I/usr/local/osdev/versions/vi/kernel/include -I/usr/local/osdev/versions/libcvv/libc/include -m64 -mno-sse -march=x86-64 -mabi=sysv -nostdlib -lgcc -std=c11
CC = /usr/local/osdev/bin/x86_64-elf-gcc
ASM = /usr/local/osdev/bin/x86_64-elf-as
LD = /usr/local/osdev/bin/x86_64-elf-ld
OBJDUMP = /usr/local/osdev/bin/x86_64-elf-objdump

export

all: $(SOURCES) $(OBJECTS)
	@echo $(OBJECTS)
	@cp -f build/*.o /usr/local/osdev/versions/vi/os_root/modules/

build/%.o: %.c 
	$(CC) -fPIC $(CFLAGS) -c $< -o ./build/$*.o1
	$(LD) -nostdlib -fPIC -shared --entry=main -o ./build/$*.o ./build/$*.o1
	objdump -x -s -d build/$*.o > ./build_temp/$*_objdump.txt
	readelf -a build/$*.o > ./build_temp/$*_elfdump.txt

clean:
	rm -rf build/*.o
	rm -rf build/*.o1
	rm -rf build_temp/*.s
	rm -rf build_temp/*.o
