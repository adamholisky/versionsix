#include <stdint.h>
#include <stddef.h>
#include "syscall.h"
#include "file.h"
#include "serial.h"
#include <device.h>
#include <process.h>
#include <debug.h>
#include <sys/types.h>

off_t lseek( int fd, off_t offset, int whence ) {
	syscall_args args = {
		.arg_1 = fd,
		.arg_2 = offset,
		.arg_3 = whence
	};

	syscall( SYSCALL_LSEEK, 3, &args );
}

extern kernel_proc_data global_proc_data;

off_t lseek_syscall_handler( registers **context, int fd, off_t offset, int whence ) {
	if( fd < 0 ) {
		return 0;
	}

	if( fd >= PROCESS_MAX_FDS ) {
		return 0;
	}
	
	if( global_proc_data.current_process->file_descriptors[fd].in_use == false ) {
		return 0;
	}

	if( whence == SEEK_SET ) {
		global_proc_data.current_process->file_descriptors[fd].pos = offset;
	} else if( whence == SEEK_CUR ) {
		global_proc_data.current_process->file_descriptors[fd].pos =+ offset;
	} else if( whence == SEEK_END ) {
		struct stat sb;

		int stat_err = vfs_getattr( global_proc_data.current_process->file_descriptors[fd].path, &sb );

		if( stat_err == -1 ) {
			return -1;
		}

		global_proc_data.current_process->file_descriptors[fd].pos = sb.st_size - offset;
	} else {
		return -1;
	}
	return 0;
}