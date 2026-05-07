#include <kernel_common.h>
#include <avs_dev_api.h>
#include <kmemory.h>

int main( int argc, char *argv[] ) {
	uint64_t size = 0;

	if( argc != 2 ) {
		printf( "Missing file. Syntax: apirun <path_to_file>\n" );
		return 1;
	}

	size = avs_dev_api_get_file_size( argv[1] );

	printf( "Got size back: %ld\n", size );

	char *data = kmalloc( size );

	size_t final_size = avs_dev_api_load_file( argv[1], data );

	printf( "Got data %ld from file back. Executing.\n", final_size );

	kdebug_peek_at( data );

	pid_t child_pid = (pid_t)syscall( SYSCALL_FORK, 0, NULL );

	//printf( "child process id: %d\n", child_pid );
	if( child_pid == 0 ) {
		//klog( LOG_DEBUG, "In child, execve-ing" );
		//kdebug_peek_at( data );
		execm( data, NULL, NULL );
	} else {
		//klog( LOG_DEBUG, "In parent, waiting for child process to exit." );
		
		wait( child_pid );

		printf( "Post wait.\n" );
	}

	kfree(data);

	return 0;
}