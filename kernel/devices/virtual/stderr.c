#include <kernel_common.h>
#include <device.h>
#include <devices/stderr.h>

device d_stderr;

void device_register_stderr( void ) {
	memset( &d_stderr, 0, sizeof(device) );

	strcpy( d_stderr.major_id, "stderr" );
	strcpy( d_stderr.minor_id, "0" );

	d_stderr.close = stderr_close;
	d_stderr.open = stderr_open;
	d_stderr.read = stderr_read;
	d_stderr.write = stderr_write;

	device_register( &d_stderr );
}

void stderr_close( inode_id id ) {
	// Intentionally blank
}

void stderr_open( inode_id id ) {
	// Intentionally blank
}

uint8_t stderr_read( inode_id id, uint8_t * buff, uint64_t count, uint64_t offset ) {	
	// Intentionally blanks
}

void stderr_write( inode_id id, void *buff, size_t count, size_t offset ) {
	// God help us if this ever fails
	device *com4 = device_get_major_minor_device( "serial", "4" );
	com4->write( id, buff, count, offset );
}