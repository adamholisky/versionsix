#include <kernel_common.h>
#include <kshell.h>

KSHELL_COMMAND( dm, kshell_app_dm_main )

int kshell_app_dm_main( int argc, char *argv[] ) {
	printf( "argc: %d\n", argc );
    
    for( int i = 0; i < argc; i++ ) {
        printf( "argv[%d]: \"%s\"\n", i, argv[i] );
    }

    printf( "returning 1\n" );

    return 1;
}