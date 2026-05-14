#include <stdio.h>

char my_string[] = {"This is my string.\n"};

int b_saved = 0;

void test_func_one( void ) {
    printf( "Test funtion one.\n" );
}

int test_func_two( int b ) {
    printf( "Test function two: %d\n", b );

    b_saved = b;

    return b;
}

char* test_func_three( int a, char *s ) {
    printf( "Test function three: %d and %d and %s\n", a, b_saved, s );

    return my_string;
}