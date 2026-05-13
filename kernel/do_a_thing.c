/**
 * @file do_a_thing.c A file to do a thing in!
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2026-05-01
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <kernel_common.h>
#include <kmemory.h>
#include <syscall.h>
#include <stdio.h>

#include <lib.h>

extern void libc_internal_initalize( void );

void do_a_thing( void ) {
	libc_internal_initalize();

	do_a_thing_main( 0, NULL );
}

int do_a_thing_main( int argc, char* argv[] ) {
	klog( LOG_INFO, "-----> doing a thing <-----" );
	debugf( "-----> doing a thing <-----\n" );

	lib_initalize();
	lib_register( "/bin/libtest.so" );

	debugf( "-----> stopped doing a thing <-----\n" );
	klog( LOG_INFO, "-----> stopped doing a thing <-----" );
	return 0;
}