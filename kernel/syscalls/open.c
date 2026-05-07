#include <kernel_common.h>
#include <syscall.h>
#include <process.h>
#include <vfs.h>

extern kernel_proc_data global_proc_data;

int open( const char *path, int flags ) {
	syscall_args args = {
		.arg_1 = path,
		.arg_2 = flags
	};

	return syscall( SYSCALL_OPEN, 2, &args );
}

int open_syscall_handler( registers **context, const char *path, int flags, umode_t mode  ) {
	// Find the file and attr it, or bail

	file_stats st;
	int ga_err = vfs_getattr( path, &st );

	if( ga_err != VFS_ERROR_NONE ) {
		return ga_err;
	}

	// Get a new FD for the process, or bail

	int new_fd = process_get_free_fd( global_proc_data.current_process );

	if( new_fd <= 0 ) {
		return new_fd;
	}

	// Get a new FD for the system, or bail

	vfs_fd *new_fd_sys = vfs_get_unused_fd();
	
	if( new_fd_sys == NULL ) {
		return 0;
	}

	// Setup the process FD info
	global_proc_data.current_process->file_descriptors[new_fd].in_use = true;
	kstrcpy( global_proc_data.current_process->file_descriptors[new_fd].path, path );
	global_proc_data.current_process->file_descriptors[new_fd].system_fd = new_fd_sys->id;

	// Setup the system FD info
	new_fd_sys->in_use = true;
	new_fd_sys->process_fd = new_fd;
	new_fd_sys->ref_count++;
}

