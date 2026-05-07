#include <kernel_common.h>
#include <syscall.h>
#include <process.h>
#include <vfs.h>

extern kernel_proc_data global_proc_data;

int close( int fd ) {
	syscall_args args = {
		.arg_1 = fd
	};

	return syscall( SYSCALL_CLOSE, 1, &args );
}

int close_syscall_handler( registers **context, int fd ) {
	if( fd >= PROCESS_MAX_FDS ) {
		klog( LOG_ERROR, "fd was %d, too big", fd );
		return -1;
	}

	if( !global_proc_data.current_process->file_descriptors[fd].in_use ) {
		klog( LOG_ERROR, "fd %d not in use", fd );
		return -1;
	}


	// Close out the process FD
	global_proc_data.current_process->file_descriptors[fd].in_use = false;
	global_proc_data.current_process->file_descriptors[fd].was_used = true;
	memset( global_proc_data.current_process->file_descriptors[fd].path, 0, 255 );
	global_proc_data.current_process->file_descriptors[fd].pos = 0;

	// Close out the system FD
	vfs_fd *v_fd = vfs_get_fd_data( global_proc_data.current_process->file_descriptors[fd].system_fd );

	if( v_fd == NULL ) {
		klog( LOG_ERROR, "v_fd is null. proc_fd=%d", fd );
		return -1;
	}

	v_fd->in_use = false;
	v_fd->inode_id = 0;
	v_fd->ref_count = 0;

	return 1;
}