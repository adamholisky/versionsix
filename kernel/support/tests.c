#include <kernel_common.h>
#include <tests.h>
#include <program.h>

void tests_run_tests( void ) {
	tests_run_program();

	do_immediate_shutdown();
}

void tests_run_program( void ) {
	program_load( "/modules/first.o" );
}