#include <kernel_common.h>
#include <process.h>

/*	First program to test in program loader
 */

char *get_hello( void );

char hello[] = "Hi, from a binary program!\n";

int main( int argc, char *argv[] ) {
	pid_t pid = 0;
	uint64_t ret = 0;

	printf( "This is immediately before fork.\n" );

	pid = (pid_t)syscall( SYSCALL_FORK, 0, NULL );
	
	printf( "This is immediately after fork.\n" );

	uint64_t yield_syscall_num = SYSCALL_SCHED_YIELD;

	switch( pid ) {
		case -1:
			printf( "Fork faid somehow?\n" );
			break;
		case 0:
			printf( "I'm the child process, fork() returned a pid of %d. My real pid is %d.\n", pid, process_get_current_proc_id() );

			while( true ) {
				//printf( "A" );
				//proc_syscall( SYSCALL_SCHED_YIELD, 0, NULL );
				__asm__	__volatile__ ( 
					"movq %1, %%rax \n"
					"int %2 \n"
					"movq %%rax, %0"
					:"=r"(ret)
					:"r"(yield_syscall_num), "i"(0xFE)
					:"%rax" 
				);
			}
			break;
		default:
			printf( "I'm the parent process, fork() returned a pid of %d. My real pid is %d.\n", pid, process_get_current_proc_id() );
			while( true ) {
				//
				//proc_syscall( SYSCALL_SCHED_YIELD, 0, NULL );
				__asm__	__volatile__ ( 
					"movq %1, %%rax \n"
					"int %2 \n"
					"movq %%rax, %0"
					:"=r"(ret)
					:"r"(yield_syscall_num), "i"(0xFE)
					:"%rax" 
				);
			}
	}

	return 69;
}

char *get_hello( void ) {
	return hello;
}