#include <kernel_common.h>
#include <syscall.h>
#include <process.h>
#include <vfs.h>

#define O_ACCMODE (03|O_SEARCH)
#define O_RDONLY  00
#define O_WRONLY  01
#define O_RDWR    02
#define O_CREAT        0100
#define O_EXCL         0200
#define O_NOCTTY       0400
#define O_TRUNC       01000
#define O_APPEND      02000
#define O_NONBLOCK    04000
#define O_DSYNC      010000
#define O_SYNC     04010000
#define O_RSYNC    04010000
#define O_DIRECTORY 0200000
#define O_NOFOLLOW  0400000
#define O_CLOEXEC  02000000

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

	int new_fd = process_alloc_fd( global_proc_data.current_process );

	if( new_fd <= 0 ) {
		return new_fd;
	}

	// Get a new FD for the system, or bail

	vfs_fd *new_fd_sys = vfs_alloc_fd();
	
	if( new_fd_sys == NULL ) {
		klog( LOG_ERROR, "Cannot allocate new vfs fd." );

		process_free_fd( global_proc_data.current_process, new_fd );

		return 0;
	}

	// Setup the process FD info
	global_proc_data.current_process->file_descriptors[new_fd].in_use = true;
	kstrcpy( new_fd_sys->path, path );
	global_proc_data.current_process->file_descriptors[new_fd].vfs_fd = new_fd_sys->id;

	// Setup the system FD info
	new_fd_sys->in_use = true;
	new_fd_sys->process_fd = new_fd;
	new_fd_sys->ref_count++;
	new_fd_sys->close_on_exec = flags & O_CLOEXEC ? true : false;
	new_fd_sys->read_only = flags & O_RDONLY ? true : false;
	new_fd_sys->write_only = flags & O_WRONLY ? true : false;

	// Return the process's new FD
	return new_fd;
}

