#include <stdio.h>
#include <avsos.h>

/*	First program to test in program loader
 */

int main( int argc, char *argv[] ) {
	pid_t pid = 0;
	uint64_t ret = 0;

	// Callback to the OS to see if we should Do A Thing
	//do_a_thing();

	printf( "In first process.\n" );

	pid = (pid_t)syscall( SYSCALL_FORK, 0, NULL );
	
	uint64_t yield_syscall_num = SYSCALL_SCHED_YIELD;

	// Do user space setup stuff here

	// Forever loops

	switch( pid ) {
		case -1:
			printf( "Fork failed somehow?\n" );
			break;
		case 0:
			printf( "Running /bin/avsshell\n" );
			execve( "/bin/avsshell", NULL, NULL );

			printf( "Done running shell? Looping.\n" );

			while( true ) {
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
			wait( pid );
			
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

	return 67;
}