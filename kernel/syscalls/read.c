#include <kernel_common.h>
#include <syscall.h>
#include <vfs.h>
#include <keyboard.h>

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
		char *s = (char *)buf;

		for( int i = 0; i < count; i++ ) {
			uint8_t scancode = keyboard_get_scancode();
			char c = keyboard_scancode_to_char( scancode );

			s[i] = c;

			ret_val = i;

			if( c == '\n' ) {
				break;
			}
		}
	} else {
		ret_val = vfs_read( global_proc_data.current_process->file_descriptors[fd].path, buf, count, global_proc_data.current_process->file_descriptors[fd].pos );
	}

	return ret_val;
}