#define AVSOS_KERNEL 1

#include <stdio.h>
#include <kernel_common.h>
#include <process.h>
#include <syscall.h>

int thread_routine( void );

int main( int argc, char **argv ) {
	printf( "Starting thread tests.\n" );
	
	printf( "Parent PID: %d\n", process_get_current_proc_id() );

	int clone_result = clone( thread_routine, 0, NULL );

	for( int j = 0; j < 10; j++ ) {
		debugf( "doing yield\n" );
		yield();

		printf( "A" );
	}
}

// force this variable to not be on the routine's stack.
int ex_status = 5;

int thread_routine( void ) {
	printf( "I'm a threaded routine.\n" );
	printf( "Thread PID: %d\n", process_get_current_proc_id() );

	for( int k = 0; k < 10; k++ ) {
		debugf( "doing yield\n" );
		yield();

		printf( "B" );
	}

	exit( ex_status );

	return 5;
}