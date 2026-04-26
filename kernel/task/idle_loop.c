#include <kernel_common.h>
#include <task.h>
#include <process.h>

uint64_t i;

int kernel_idle_loop_entry( int argc, char *argv[] ) {
	kernel_idle_loop();
}

void kernel_idle_loop( void ) {
    i = 0;

	klog( LOG_INFO, "In idle loop." );

	do {
		i++; // Keep track of how many times we loop

		klog( LOG_DEBUG, "Idle Count: %d", i );

		syscall( SYSCALL_SCHED_YIELD, 0, NULL );
	} while( 1 );
}

uint64_t get_i( void ) {
	return i;
}