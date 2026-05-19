.DEFAULT_GOAL := all

ROOT_DIR = $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

DEFINES = -DVIOS

CC = /usr/local/osdev/bin/x86_64-elf-avsos-gcc
LD = /usr/local/osdev/bin/x86_64-elf-avsos-ld
ASM = /usr/local/osdev/bin/x86_64-elf-avsos-as
OBJDUMP = /usr/local/osdev/bin/x86_64-elf-avsos-objdump

BUILD_LOG = $(ROOT_DIR)/build_temp/build.log
SHELL :=/bin/bash -O globstar
VPATH = $(shell find ./src -type d -printf "src/%P:")

SOURCES_C = $(shell ls src/**/*.c)
OBJECTS_C = $(patsubst %.c, build/%.o, $(shell ls src/**/*.c | xargs -n 1 basename))
EXECS_C = $(patsubst %.c, full/%, $(shell ls src/**/*.c | xargs -n 1 basename))

CFLAGS_REDUCED = $(DEFINES) -Wno-write-strings \
	-Wno-pointer-to-int-cast \
	-Wno-discarded-qualifiers \
	-Wno-int-conversion \
	-Wno-incompatible-pointer-types \
	-ffreestanding \
	-O0 \
	-g \
	-I/usr/local/osdev/source/avs-libc/include \
	-I$(ROOT_DIR)/../liblua-static/include \
    -m64                 \
    -march=x86-64        \
    -mabi=sysv           \
	-std=c11 \
	-lgcc \
	-nostdlib

CFLAGS_APP_SPECIFIC = -fPIC 

.PHONY: glue

all: install 
#glue/crt0.o $(OBJECTS_C) $(EXECS_C)

#-Wl,--unresolved-symbols=ignore-all 

finished/%.exec: build/%.o
	@$(eval OBJNAME := $(shell basename $@))
	@>&2 printf "[Link] $(OBJNAME)\n"
	$(LD) -shared -E -Bsymbolic -z max-page-size=0x1000 -o finished/$(OBJNAME) glue/crt0.o $<  >> $(BUILD_LOG)
	readelf -W -a finished/$(OBJNAME) > build_temp/elfdump-$(OBJNAME).txt
	$(OBJDUMP) -x -D -S finished/$(OBJNAME) > build_temp/objdump-$(OBJNAME).txt

full/%: build/%.o 
	@$(eval OBJNAME := $(shell basename $@))
	@>&2 printf "[Link] $(OBJNAME)\n"
	/usr/local/osdev/bin/x86_64-elf-avsos-gcc -g -lc -o full/$(OBJNAME) $< 
	readelf -W -a full/$(OBJNAME) > build_temp/elfdump-FULL-$(OBJNAME).txt
	$(OBJDUMP) -x -D -S full/$(OBJNAME) > build_temp/objdump-FULL-$(OBJNAME).txt

build/%.o: %.c
	@$(eval OBJNAME := $(shell basename $@))
	@>&2 printf "[Compile] $(OBJNAME)\n"
	$(CC) $(CFLAGS_REDUCED) $(CFLAGS_APP_SPECIFIC) -c $< -o build/$(OBJNAME) >> $(BUILD_LOG)

glue:
	@>&2 printf "[Build] Glue\n"
	$(CC) $(CFLAGS_REDUCED) $(CFLAGS_APP_SPECIFIC) -c glue/crt0.S -o glue/crt0.o >> $(BUILD_LOG)
	$(CC) $(CFLAGS_REDUCED) $(CFLAGS_APP_SPECIFIC) -c glue/crti.S -o glue/crti.o >> $(BUILD_LOG)
	$(CC) $(CFLAGS_REDUCED) $(CFLAGS_APP_SPECIFIC) -c glue/crtn.S -o glue/crtn.o >> $(BUILD_LOG)
	cp -f glue/*.o /usr/local/osdev/lib/gcc/x86_64-elf-avsos/15.2.0
	cp -f glue/crt0.o /usr/local/osdev/lib/gcc/x86_64-elf-avsos/15.2.0/crt1.o

apps: $(EXEC_C)

install:
	@make install_stage2 >> $(BUILD_LOG)
	@>&2 printf "[Install] Done\n"

install_stage2: $(EXECS_C)
	@make $(EXECS_C)
	cd /home/adam/code/afs-fuse && sudo ./asvfs-fuse --image=/usr/local/osdev/source/versionsix/asvfs_drive.img testmnt && sudo cp -f /avsos/lib/libc.so testmnt/libc.so && sudo cp -f /usr/local/osdev/source/versionsix/core_apps/full/* testmnt/bin && sudo fusermount -u testmnt

clean:
	rm -rf build/*.o
	rm -rf build_temp/*
	rm -rf finished/*
	rm -rf full/*

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


# /usr/local/osdev/bin/x86_64-elf-avsos-gcc -DVIOS -DAVSOS_KERNEL -Wno-write-strings -Wno-pointer-to-int-cast -Wno-discarded-qualifiers -Wno-int-conversion -Wno-incompatible-pointer-types -O0 -g -I/usr/local/osdev/source/versionsix/core_apps/../kernel/include -I/usr/local/osdev/source/versionsix/core_apps/../avs-libc-static/include -I/usr/local/osdev/source/versionsix/core_apps/../liblua-static/include -I/usr/local/osdev/source/versionsix/core_apps/src -m64 -march=x86-64 -mabi=sysv -std=c11 -lgcc -fPIC  -c ../src/ls.c -o ls.o

# /usr/local/osdev/bin/x86_64-elf-avsos-gcc -Wl,--unresolved-symbols=ignore-all ls.o -o ls