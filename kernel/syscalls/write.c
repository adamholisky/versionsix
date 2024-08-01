#include <stdint.h>
#include <stddef.h>
#include "syscall.h"
#include "file.h"
#include "serial.h"
#include <device.h>

extern void main_console_putc( char c );

device *stderr_dev = NULL;

size_t write(int fd, void *buff, size_t count) {
	char *char_buff = (char *)buff;
	char *char_buff_end = (char *)buff + count;

	if (fd == FD_STDOUT) {
		while (char_buff != char_buff_end) {
			main_console_putc( *char_buff );
			char_buff++;
		}
	}

	if (fd == FD_STDERR) {
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
	}
}