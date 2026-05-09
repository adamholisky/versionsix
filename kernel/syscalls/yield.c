#include <syscall.h>
#include <process.h>

void yield( void ) {
	syscall( SYSCALL_SCHED_YIELD, 0, NULL );	
}