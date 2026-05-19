#include <stdio.h>
#include <stdlib.h>

int main( int argc, char *argv[] ) {
	if( argc != 2 ) {
		printf( "Syntax error. Missing filename. cat <filename>\n" );
		return 1;
	}

	FILE *f = fopen( argv[1], "r" );

	if( f == NULL ) {
		printf( "Could not open/find %s\n.", argv[1] );
		return 1;
	}

	struct stat s;

	fstat( f->fd, &s );

	char *data = malloc( s.st_size );

	if( data == NULL ) {
		printf( "data is null. exiting.\n" );
		return 1;
	}

	printf( "size: %d\n", s.st_size );

	fread( data, 1, s.st_size, f );

	char *c = *data;

	while( c < data + s.st_size ) {
		printf( "%s", c );
		c = c + 80;
	}
}