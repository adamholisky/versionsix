#include <stdio.h>

#include <avs_dev_api.h>

#include <lualib.h> 
#include <lauxlib.h>

lua_State *L;

int main( int argc, char **argv ) {	
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

	return 0;
}