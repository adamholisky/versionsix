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

#include <avs_dev_api.h>

#include <lualib.h> 
#include <lauxlib.h>

extern void libc_internal_initalize( void );

void do_a_thing( void ) {
	libc_internal_initalize();

	do_a_thing_main( 0, NULL );
}

lua_State *L;

int do_a_thing_main( int argc, char* argv[] ) {
	klog( LOG_INFO, "-----> doing a thing <-----" );
	debugf( "-----> doing a thing <-----\n" );

	L = luaL_newstate();

	/* Check the return value */
	if ( L == NULL ) {
		fprintf( stdout, "Lua: cannot initialize\n" );
		return -1;
	} else {
		fprintf( stderr, "Lua initalized!\n" );
	}

	luaL_openlibs(L);

	int lua_err = luaL_dostring( L, "print(\"Hello, from lua!\");" );

	printf( "lua returned: %d\n", lua_err );
	if( lua_err != 0 ) {
		printf( "lua says: %s\n", lua_tostring(L, 1) );
	}

	lua_close(L);

	debugf( "-----> stopped doing a thing <-----\n" );
	klog( LOG_INFO, "-----> stopped doing a thing <-----" );
	return 0;
}