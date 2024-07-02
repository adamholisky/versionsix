#include <stdint.h>
#include <stddef.h>
#include "syscall.h"
#include "file.h"
#include "serial.h"

extern void main_console_putc( char c );

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
		int com_port = COM4;

		while (char_buff != char_buff_end) {
			serial_write_port(*char_buff, com_port);
			char_buff++;
		}
	}
}