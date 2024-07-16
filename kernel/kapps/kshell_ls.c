#include <kernel_common.h>
#include <kshell_app.h>
#include <fs.h>
#include <afs.h>

KSHELL_COMMAND( ls, kshell_app_ls_main )

char empty_string[] = "";

int kshell_app_ls_main( int argc, char *argv[] ) {
	printf( "In main.\n" );
	char *path = NULL;

	if( argc == 2 ) {
		path = argv[1];
	} else {
		path = empty_string;
	}

	primative_ls( path );

	printf( "Ending.\n" );
	return 0;
}