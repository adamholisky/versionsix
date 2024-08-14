#include <kernel_common.h>
#include <tests.h>
#include <program.h>
#include <task.h>

void tests_run_tests( void ) {
	tests_run_program();

	/* debugf( "End of run tests. Shutting down.\n" );
	do_immediate_shutdown(); */
}

void tests_run_program( void ) {
	program *p = program_load( "/modules/first.o" );
	task_create_from_program( p );
}