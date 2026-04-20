#include <kernel_common.h>
#include <device.h>
#include <devices/stdout.h>
#include <keyboard.h>

device stdout;

void device_register_stdout( void ) {
	memset( &stdout, 0, sizeof(device) );

	strcpy( stdout.major_id, "stdout" );
	strcpy( stdout.minor_id, "0" );

	stdout.close = stdout_close;
	stdout.open = stdout_open;
	stdout.read = stdout_read;
	stdout.write = stdout_write;

	device_register( &stdout );
}

void stdout_close( inode_id id ) {
	// Intentionally blank
}

void stdout_open( inode_id id ) {
	// Intentionally blank
}

uint8_t stdout_read( inode_id id, uint8_t * buff, uint64_t count, uint64_t offset ) {
	uint8_t scancode = 0;
	char c = 0;
	
	#ifdef STDIO_SERIAL_3
		device *com3 = device_get_major_minor_device( "serial", "3" );
		scancode = com3->read( id, buff, count, offset );
		c= keyboard_scancode_to_char( scancode );
	#else 
		scancode = keyboard_get_scancode();
		c = keyboard_scancode_to_char( scancode );	// this checks for scancode under 0x81, otherwise returns 0
	#endif

	return c;
}

void stdout_write( inode_id id, void *buff, size_t count, size_t offset ) {
	#ifdef STDIO_SERIAL_3
		device *com3 = device_get_major_minor_device( "serial", "3" );
		com3->write( 0, buff, count, offset );
	#else
	#endif
}