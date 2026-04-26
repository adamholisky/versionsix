#include <kernel_common.h>
#include <kshell_app.h>

KSHELL_COMMAND( tasktests, kshell_app_tasktests_main )

int kshell_app_tasktests_main( int argc, char *argv[] ) {
	printf( "In main.\n" );

	bool div_zero = false;

    if( argc > 1 ) {
		for( int i = 1; i < argc; i++ ) {
			if( strcmp( argv[i], "div0" ) == 0 ) {
				div_zero = true;
			}
		}
	}

	if( div_zero ) {
		do_divide_by_zero();
	}

    printf( "Ending.\n" );
	return 0;
}