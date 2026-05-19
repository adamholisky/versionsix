#include <kernel_common.h>
#include <kmemory.h>
#include <syscall.h>
#include <stdio.h>

#include <setjmp.h>

jmp_buf jump_buffer;

void my_function( void );

int main( int argc, char **argv ) {
	if( setjmp( jump_buffer ) == 1 ) {
		printf( "We jumped back to main.\n" );
	} else {
		printf( "We're going to call my_function() now.\n" );

		my_function();
	}

	printf( "Ending from main.\n" );

	return 0;
}

void my_function( void ) {
	int x;

	while( true ) {
		x++;

		if( x == 3 ) {
			printf( "Condition met. Jumping back.\n" );
			longjmp( jump_buffer, 1 );
		}
	}
}