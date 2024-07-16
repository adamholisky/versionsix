#include <kernel_common.h>
#include <kshell_app.h>
#include <fs.h>
#include <afs.h>

KSHELL_COMMAND( cat, kshell_app_cat_main )

int kshell_app_cat_main( int argc, char *argv[] ) {
	char *path = NULL;

	if( argc == 2 ) {
		path = argv[1];
	} else {
		printf( "Missing filename.\n" );
        return 1;
	}

	primative_cat( path );

	return 0;
}