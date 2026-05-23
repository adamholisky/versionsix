#include <kernel_common.h>
#include <device.h>
#include <devices/stdout.h>
#include <keyboard.h>
#include <vui/vui.h>
#include <vui/console.h>

device d_stdout;
device *d_use_as_stdout;

void device_register_stdout( void ) {
	memset( &d_stdout, 0, sizeof(device) );

	strcpy( d_stdout.major_id, "stdout" );
	strcpy( d_stdout.minor_id, "0" );

	d_stdout.close = stdout_close;
	d_stdout.open = stdout_open;
	d_stdout.read = stdout_read;
	d_stdout.write = stdout_write;

	device_register( &d_stdout );
	d_use_as_stdout = NULL;
}

void stdout_close( inode_id id ) {
	// Intentionally blank
}

void stdout_open( inode_id id ) {
	// Intentionally blank
}

uint8_t stdout_read( inode_id id, uint8_t * buff, uint64_t count, uint64_t offset ) {
	return 0;
}

extern vui_console* main_console;
void stdout_write( inode_id id, void *buff, size_t count, size_t offset ) {
	#ifdef STDIO_SERIAL_3
		device *com3 = device_get_major_minor_device( "serial", "3" );
		com3->write( 0, buff, count, offset );
	#else
	#endif

	if( d_use_as_stdout == NULL ) {
		for( int i = 0; i < count; i++ ) {
			char *c_ptr = (char *)buff;
			char c = *c_ptr;
			vui_console_put_char( main_console, c );
		}
	} else {
		d_use_as_stdout->write( id, buff, count, offset );
	}
}

void stdout_set_redirect( device *redirect_to ) {
	klog( LOG_INFO, "stdout redirected to: %s:%s", redirect_to->major_id, redirect_to->minor_id );
	d_use_as_stdout = redirect_to;
}