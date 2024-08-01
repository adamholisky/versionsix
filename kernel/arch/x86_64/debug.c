/**
 * @file debug.c
 * @author Adam Holisky
 * @brief Debug helper function
 * @version 0.1
 * @date 2024-05-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <stddef.h>
#include <kernel_common.h>
#include <string.h>
#include <printf.h>
#include "syscall.h"
#include "file.h"
#include <debug.h>

char debugf_buff[4096];

void debugf_stage2( char * message, ... ) {
    va_list args;
    int len = 0;

    va_start( args, message );
	len = vsnprintf( debugf_buff, 1024, message, args ); 
	va_end( args );

    write( FD_STDERR, debugf_buff, len );
}

void do_divide_by_zero( void ) {
    __asm__ volatile( 
			"mov $0, %rax \n\t"
			"mov $0, %rcx \n\t"
			"div %rcx \n\t"
		);

    printf( "divide by zero done.\n" );
}

void kdebug_peek_at( uint64_t addr ) {
    kdebug_peek_at_n( addr, 10 );
}

char * kdebug_peek_at_n( uint64_t addr, int n ) {
	debugf_raw( "\nLooking at 0x%16llX\n", addr );

	uint8_t *ptrn = (uint8_t *)addr;
	char *ptr = (char *)addr;

	debugf_raw( "                      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  Decoded Text\n" );

	for( int x = 0; x < n; x++ ) {
		debugf_raw( "0x%16llX    %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X  %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c %c\n",
		addr + (x * 0xF), 
		*(ptrn + 0), *(ptrn + 1), *(ptrn + 2), *(ptrn + 3), *(ptrn + 4), *(ptrn + 5), *(ptrn + 6), *(ptrn + 7), *(ptrn + 8), *(ptrn + 9), *(ptrn + 10), *(ptrn + 11), *(ptrn + 12), *(ptrn + 13), *(ptrn + 14), *(ptrn + 15),
		peek_char( *(ptr + 0) ), peek_char( *(ptr + 1) ), peek_char( *(ptr + 2) ), peek_char( *(ptr + 3) ), peek_char( *(ptr + 4) ), peek_char( *(ptr + 5) ), peek_char( *(ptr + 6) ), peek_char( *(ptr + 7) ), peek_char( *(ptr + 8) ), peek_char( *(ptr + 9) ), peek_char( *(ptr + 10) ), peek_char( *(ptr + 11) ), peek_char( *(ptr + 12) ), peek_char( *(ptr + 13) ), peek_char( *(ptr + 14) ), peek_char( *(ptr + 15) ) );

		ptr = ptr + 0x10;
		ptrn = ptrn + 0x10;
	}

	debugf_raw( "\n" );

	return "KDEBUG_RETURN";
}

char peek_char( char c ) {
	if( (c > 31) && (c < 127) ) {
		return c;
	} else {
		return '.';
	}
}
