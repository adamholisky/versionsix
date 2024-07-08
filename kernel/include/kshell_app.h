#ifndef VIOS_KSHELL_APP_INCLUDED
#define VIOS_KSHELL_APP_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

void kshell_add_command( char *command_name, void *main_function );

#define KSHELL_COMMAND( name, main_function ) \
	int kshell_app_ ##name## _main( int c, char *argv[] ); \
	void kshell_app_add_command_ ##name ( void ) { \
		kshell_add_command( #name, (void *)main_function ); \
	}

#ifdef __cplusplus
}
#endif
#endif