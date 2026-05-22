#include <kernel_common.h>
#include <syscall.h>
#include <vfs.h>
#include <keyboard.h>
#include <device.h>

extern kernel_proc_data global_proc_data;

size_t read( int fd, void *buf, size_t count ) {
	syscall_args args = {
		.arg_1 = fd,
		.arg_2 = buf,
		.arg_3 = count
	};

	return syscall( SYSCALL_READ, 3, &args );
}


int read_syscall_handler( registers **context, int fd, void *buf, size_t count ) {
	int ret_val = 0;

	if( fd < 0 ) {
		klog( "fd out of bounds: %d", fd );
		return -1;
	}

	if( fd >= PROCESS_MAX_FDS ) {
		klog( "fd out of bounds: %d", fd );
		return -1;
	}

	if( !global_proc_data.current_process->file_descriptors[fd].in_use ) {
		klog( "fd not in use: %d", fd );
		return -1;
	}

	if( fd == STDIN_FILENO ) {
		device *dev = device_get_major_minor_device( "stdin", "0" );
		dev->read( fd, buf, count, 0 );
	} else {
		vfs_fd *v_fd = vfs_get_fd_data( global_proc_data.current_process->file_descriptors[fd].vfs_fd );

		if( v_fd->write_only ) {
			klog( "fd was opened in write only. cannot read. fd=%d   path=%s", fd, v_fd->path );
		}
		
		ret_val = vfs_read( v_fd->path, buf, count, global_proc_data.current_process->file_descriptors[fd].pos );
	}

	return ret_val;
}