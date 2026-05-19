#include <stdio.h>
#include "testlib.h"

int main( int argc, char *argv[] ) {
	printf( "In shared library test.\n" );

	test_func_one();

	int test_two_ret = test_func_two( 3 );
	printf( "test_two_ret = %d\n", test_two_ret );

	char *test_three_ret = test_func_three( 5, "Maddox" );
	printf( "test_three_ret = %s\n", test_three_ret );

	printf( "Out shared library test.\n" );
}