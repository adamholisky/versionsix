.DEFAULT_GOAL := all

ROOT_DIR=/usr/local/osdev/source/versionsix

CC = /usr/local/osdev/bin/x86_64-elf-gcc
ASM = /usr/local/osdev/bin/x86_64-elf-as
LD = /usr/local/osdev/bin/x86_64-elf-ld
OBJDUMP = /usr/local/osdev/bin/x86_64-elf-objdump

SHELL :=/bin/bash -O globstar
VPATH = $(shell find ./src -type d -printf "src/%P:")

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
	-std=c11 \
	-Wl,--export-dynamic \
	-lgcc \
	-fPIC

all: finished/first.exec finished/ls.exec
	
finished/first.exec: build/crt0.o
	$(CC) $(CFLAGS) -c src/first.c -o build/first_core.o 
	$(LD) -nostdlib -fPIC -shared --entry=_start -z max-page-size=0x1000 -o finished/first.exec build/crt0.o build/first_core.o 
	objdump -x -s -d finished/first.exec > build_temp/first_objdump.txt
	readelf -a finished/first.exec > build_temp/first_elfdump.txt

finished/ls.exec: build/crt0.o
	$(CC) $(CFLAGS) -c src/ls.c -o build/ls.o 
	$(LD) -nostdlib -fPIC -shared --entry=_start -z max-page-size=0x1000 -o finished/ls.exec build/crt0.o build/ls.o 
	objdump -x -s -d finished/ls.exec > build_temp/ls_objdump.txt
	readelf -a finished/ls.exec > build_temp/ls_elfdump.txt

build/crt0.o:
	$(CC) $(CFLAGS) -c src/crt0.S -o build/crt0.o 

clean:
	rm -rf build/*.o
	rm -rf build/first
	rm -rf build_temp/*
	rm -rf finished/*

install:
	cd /home/adam/code/afs-fuse && sudo ./asvfs-fuse --image=/usr/local/osdev/source/versionsix/asvfs_drive.img testmnt && sudo cp -f /usr/local/osdev/source/versionsix/modules/finished/* testmnt/bin && sudo fusermount -u testmnt

