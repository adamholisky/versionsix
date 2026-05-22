#include <stdint.h>
#include <stddef.h>
#include "syscall.h"
#include "file.h"
#include <devices/serial.h>
#include <device.h>
#include <kernel_common.h>

extern kernel_proc_data global_proc_data;

extern void main_console_putc( char c );

device *stderr_dev = NULL;

size_t write( int fd, void *buff, size_t count ) {
	syscall_args args = {
		.arg_1 = fd,
		.arg_2 = buff,
		.arg_3 = count
	};

	return syscall( SYSCALL_WRITE, 3, &args );
}

void write_syscall_handler( registers **context, int fd, void *buff, size_t count ) {
	char *char_buff = (char *)buff;
	char *char_buff_end = (char *)buff + count;

	if (fd == FD_STDOUT) {
		while (char_buff != char_buff_end) {
			klog( LOG_DEBUG, "wc: '%c'", *char_buff );
			main_console_putc( *char_buff );
			char_buff++;
		}
	} else if (fd == FD_STDERR) {
		if( stderr_dev == NULL ) {
			if( devices_setup() ) {
				stderr_dev = device_get_major_minor_device( "stderr", "0" );
			}
		}
		
		if( stderr_dev ) {
			stderr_dev->write( 0, buff, count, 0 );
		} else {
			int com_port = COM4;

			while (char_buff != char_buff_end) {
				serial_write_port(*char_buff, com_port);
				char_buff++;
			}
		}
	} else {
		vfs_fd *v_fd = vfs_get_fd_data( global_proc_data.current_process->file_descriptors[fd].vfs_fd );

		if( v_fd->read_only ) {
			klog( "fd was opened in read only. cannot write. fd=%d   path=%s", fd, v_fd->path );
		}

		klog( LOG_ERROR, "Unimplemented write destination. FD=%d   buff=0x%011llX  count=%d", fd, buff, count );
		klog( LOG_ERROR, "\'%s\'", buff );
	}
}