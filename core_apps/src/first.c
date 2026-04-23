#include <kernel_common.h>

/*	First program to test in program loader
 */

char *get_hello( void );

char hello[] = "Hi, from a binary program!\n";

int main( int argc, char *argv[] ) {
	printf( "Hello, world!\n" );
	printf( "And\n" );
	printf( "%s", get_hello() );

	return 69;
}

char *get_hello( void ) {
	return hello;
}