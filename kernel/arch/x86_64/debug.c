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
#include <process.h>
#include <interrupt.h>

#define KLOG_SERIAL_PORT 0x2F8 // COM2

char debugf_buff[4096];
char klog_message[1024];
char klog_buff[4096];
char start_buff[1024];
char end_buff[1024];

char *klog_levels[] = {
	"Info",
	"Debug",
	"Error",
	"Panic"
};


#ifdef KLOG_AVS_DEV_API_OUT
/* Sending klog output to a serial port, which is then configured to send to some json ingest service. Initial development was done on elastic's stack.

Raw ingest:

curl -X POST "republic.marsdev.io:50000" -H 'Content-Type: application/json' -d '{ "level": "PANIC", file_name: "main.c", function_name: "do_things_in_a_func()", line_number: "2033", message: "This is output. It has stuff! Like \"quotes\" that have been escaped. yay?" }'

ES API ingest

POST /klog/_doc HTTP/1.1\nHost: republic.marsdev.io\nContent-Type: application/json\nContent-Length: 45\n\n{ "level": "PANIC", file_name: "main.c", function_name: "do_things_in_a_func()", line_number: "2033", message: "This is output. It has stuff! Like \"quotes\" that have been escaped. yay?" }
 */

void klog_write_str_to_serial_port( char *s, int len );
void klog_string_escape_json( char *unescaped_str, char *escaped_str );

char klog_escaped_str[4096];

void klog_stage2( int level, char *file_name, char *function_name, int line_number, char * message, ... ) {
	va_list args;
	int len = 0;
	
	memset( klog_buff, 0, 4096 );
	memset( klog_message, 0, 4096 );
	memset( klog_escaped_str, 0, 4096 );
	memset( start_buff, 0, 1024 );
	memset( end_buff, 0, 1024 );

	va_start( args, message );
	vsnprintf( klog_message, 1024, message, args );
	va_end( args );

	klog_string_escape_json( klog_message, klog_escaped_str );

	sprintf( start_buff, "{ \"level\": \"%s\", \"file\": \"%s\", \"function\": \"%s\", \"line_number\": \"%d\", \"message\": \"", klog_levels[level], file_name, function_name, line_number );
	sprintf( end_buff, "\" }\n" );

	int start_len = kstrlen(start_buff);
	char *msg_start = klog_buff + start_len;

	kstrncpy( klog_buff, start_buff, kstrlen(start_buff) );
	kstrncat( klog_buff, klog_escaped_str, kstrlen(klog_escaped_str) );
	kstrncat( klog_buff, end_buff, kstrlen(end_buff) );

	klog_write_str_to_serial_port( klog_buff, kstrlen( klog_buff ) );
}

void klog_crash( uint64_t addr, void *p ) {
	process_data *pd = p;

	memset( klog_buff, 0, 4096 );
	memset( klog_message, 0, 4096 );
	memset( klog_escaped_str, 0, 4096 );
	memset( start_buff, 0, 1024 );
	memset( end_buff, 0, 1024 );

	sprintf( start_buff, "{ \"crash\": { \"address\": \"0x%016llX\", \"path\": \"%s\", \"payload\": \"", addr, pd->path );
	sprintf( end_buff, "\" } }\n" );

	kstrncpy( klog_buff, start_buff, kstrlen(start_buff) );
	kstrncat( klog_buff, end_buff, kstrlen(end_buff) );

	klog_write_str_to_serial_port( klog_buff, kstrlen(klog_buff) );
}

void klog_write_str_to_serial_port( char *s, int len ) {
	for( int i = 0; i < len; i++ ) {
		while((inportb(KLOG_SERIAL_PORT + 5) & 0x20) == 0) {
			;
		}

		outportb( KLOG_SERIAL_PORT, *s++ );
	}
}

void klog_string_escape_json( char *unescaped_str, char *escaped_str ) {
	char *s = unescaped_str;
	int esc_index = 0;

	while(*s) {
		switch( *s ) {
			case '"':
				escaped_str[esc_index] = '\\';
				esc_index++;
				escaped_str[esc_index] = '"';
				break;
			case '\n':
				escaped_str[esc_index] = '\\';
				esc_index++;
				escaped_str[esc_index] = 'n';
				break;
			default:
				escaped_str[esc_index] = *s;
		}

		esc_index++;
		s++;
	}
}

#else
void klog_stage2( int level, char *file_name, char *function_name, int line_number, char * message, ... ) {
	va_list args;
	int len = 0;
	
	/* if( strcmp(file_name, "kernel/syscalls/execve.c") != 0 ) {
		if( level != LOG_PANIC ) {
			return;
		}
	} */
	
	memset( klog_buff, 0, 4096 );
	memset( klog_message, 0, 4096 );
	memset( start_buff, 0, 1024 );
	memset( end_buff, 0, 1024 );

	va_start( args, message );
	vsnprintf( klog_message, 1024, message, args );
	va_end( args );

	sprintf( start_buff, "<LogEntry level=\"%s\" file=\"%s\" function=\"%s\" line_number=%d >", klog_levels[level], file_name, function_name, line_number );
	sprintf( end_buff, "</LogEntry>\n" );

	int start_len = kstrlen(start_buff);
	char *msg_start = klog_buff + start_len;

	kstrncpy( klog_buff, start_buff, kstrlen(start_buff) );
	kstrncat( klog_buff, klog_message, kstrlen(klog_message) );
	kstrncat( klog_buff, end_buff, kstrlen(end_buff) );

	write( FD_STDERR, klog_buff, kstrlen( klog_buff ) );
}
#endif



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
		addr + (x * 0x10), 
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
