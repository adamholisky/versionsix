#include <bootstrap.h>
#include "syscall.h"
#include "file.h"
#include "kmemory.h"

/**
 * @brief Read a line, terminated by return
 * 
 * @return char* 
 */
char * k_bs_line_read() {
	return 0;
}

void term_put_char( char c ) {
    write( FD_STDOUT, &c, 1 );
}

void delay( uint32_t count ) {
	for( uint32_t x = 0; x < count; x++ ) {
		__asm__ __volatile__ ( "nop" );
	}
}

int kstrlen( char *s ) {
	int len = 0;

	while( *(s++) ) {
		len++;
	}

	return len;
}