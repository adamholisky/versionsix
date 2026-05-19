#include <stdio.h>
#include <kernel_common.h>

int main( int argc, char *argv[] ) {
	klog( LOG_INFO, "Shutting down via command line." );
	printf( "Shutting down via command line.\n" );
	debugf( "Shutting down via command line.\n" );

	do_immediate_shutdown();
}