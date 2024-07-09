#include <kernel_common.h>
#include <kshell_app.h>

KSHELL_COMMAND( tasktests, kshell_app_tasktests_main )

int kshell_app_tasktests_main( int argc, char *argv[] ) {
	printf( "In main.\n" );

    

    printf( "Ending.\n" );
	return 0;
}