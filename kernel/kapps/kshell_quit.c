#include <kernel_common.h>
#include <kshell_app.h>

KSHELL_COMMAND( quit, kshell_app_quit_main )

int kshell_app_quit_main( int argc, char *argv[] ) {
	debugf( "Ending via quit command.\n" );
	printf( "Ending via quit command.\n" );

	do_immediate_shutdown();
}