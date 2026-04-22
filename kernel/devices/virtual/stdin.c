#include <kernel_common.h>
#include <device.h>
#include <devices/stdin.h>
#include <keyboard.h>

device stdin;

void device_register_stdin( void ) {
	memset( &stdin, 0, sizeof(device) );

	strcpy( stdin.major_id, "stdin" );
	strcpy( stdin.minor_id, "0" );

	stdin.close = stdin_close;
	stdin.open = stdin_open;
	stdin.read = stdin_read;
	stdin.write = stdin_write;

	device_register( &stdin );
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
	
	#ifdef STDIO_SERIAL_3
		device *com3 = device_get_major_minor_device( "serial", "3" );
		c = com3->read( id, buff, count, offset );
		//c = keyboard_scancode_to_char( scancode );
		//debugf( "got c: %c (%d)", c, (int)c );
		*buff = c;
	#else 
		scancode = keyboard_get_scancode();
		c = keyboard_scancode_to_char( scancode );	// this checks for scancode under 0x81, otherwise returns 0
	#endif

	return c;
}

void stdin_write( inode_id id, void *buff, size_t count, size_t offset ) {
	// Intentionally blank
}