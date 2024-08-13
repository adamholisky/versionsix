#include <kernel_common.h>

/*	First program to test in program loader

	Module compile:

	/usr/local/osdev/bin/x86_64-elf-gcc -fPIC -DVIFS_OS_ENV -DVI_ENV_OS -Wno-write-strings -Wno-pointer-to-int-cast -Wno-discarded-qualifiers -ffreestanding -fno-omit-frame-pointer -fno-lto -fno-stack-protector -fno-stack-check -mno-red-zone -O0 -I/usr/local/osdev/versions/vi/kernel/include -I/usr/local/osdev/versions/vi/../libcvv/libc/include -m64 -mno-sse -march=x86-64 -mabi=sysv -nostdlib -lgcc -std=c11 -c first.c -o build/first.o && cp build/first.o ../os_root/modules/first.o
 */

char *get_hello( void );

int main( int argc, char *argv[] ) {
	printf( "Helll, world!\n" );

	return 0;
}

char hello[] = "Hi, from a module!";

char *get_hello( void ) {
	return hello;
}