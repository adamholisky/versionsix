#include <kernel_common.h>
#include <device.h>
#include <devices/stdin.h>
#include <keyboard.h>

device d_stdin;
device *d_use_as_stdin;

void device_register_stdin( void ) {
	memset( &d_stdin, 0, sizeof(device) );

	strcpy( d_stdin.major_id, "stdin" );
	strcpy( d_stdin.minor_id, "0" );

	d_stdin.close = stdin_close;
	d_stdin.open = stdin_open;
	d_stdin.read = stdin_read;
	d_stdin.write = stdin_write;

	#ifdef STDIO_SERIAL_3
		d_use_as_stdin = device_get_major_minor_device( "serial", "3" );
	#else
		d_use_as_stdin = device_get_major_minor_device( "ps2keyboard", "0" );
	#endif

	device_register( &d_stdin );
}

void stdin_close( inode_id id ) {
	// Intentionally blank
}

void stdin_open( inode_id id ) {
	// Intentionally blank
}

uint8_t stdin_read( inode_id id, uint8_t * buff, uint64_t count, uint64_t offset ) {
	uint8_t scancode = 0;
	char c = 0;

	c = d_use_as_stdin->read( id, buff, count, offset );

	*buff = c;

	return c;
}

void stdin_write( inode_id id, void *buff, size_t count, size_t offset ) {
	// Intentionally blank
}