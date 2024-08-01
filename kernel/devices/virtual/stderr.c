#include <kernel_common.h>
#include <device.h>
#include <devices/stderr.h>

device stderr;

void device_register_stderr( void ) {
	memset( &stderr, 0, sizeof(device) );

	strcpy( stderr.major_id, "stderr" );
	strcpy( stderr.minor_id, "0" );

	stderr.close = stderr_close;
	stderr.open = stderr_open;
	stderr.read = stderr_read;
	stderr.write = stderr_write;

	device_register( &stderr );
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