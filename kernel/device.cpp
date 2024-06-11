#include <kernel_common.h>
#include <kmemory.h>
#include <device.h>

Device::Device() {
	printf( "In const Device\n" );
}

Device::~Device() {
	printf( "In dest\n" );
}

void Device::open( void ) {
	printf( "Device open\n" );
}

void Device::close( void ) {

}

void Device::read( void ) {

}

void Device::write( void ) {

}

void device_initalize( void ) {
	/* SerialDevice *s;
	TerminalDevice *t;
	Device *d;

	printf( "Serial new\n" );
	s = new SerialDevice();

	printf( "\nTerminal new\n" );
	t = new TerminalDevice();

 	printf( "\nCasting\n" );
	d = s;
	d->open();
	d = t;
	d->open(); */
}