#include <stdio.h>
#include "testlib.h"
#include <kernel_common.h>

int main( int argc, char *argv[] ) {
	klog( LOG_DEBUG, "In shared library test.\n" );

	test_func_one();

	int test_two_ret = test_func_two( 3 );
	printf( "test_two_ret = %d\n", test_two_ret );

	char *test_three_ret = test_func_three( 5, "Maddox" );
	printf( "test_three_ret = %s\n", test_three_ret );

	klog( LOG_DEBUG, "Out shared library test.\n" );
}