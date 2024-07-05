#include <kernel_common.h>
#include <limine_bootstrap.h>
#include <limine.h>
#include <serial.h>
#include <acpi.h>
#include <file.h>
#include <interrupt.h>
#include <timer.h>
#include <page.h>
#include <kmemory.h>
#include <kshell.h>
#include <ksymbols.h>
#include <pci.h>
#include <e1000.h>
#include <task.h>
#include <elf.h>
#include <gui/console.h>
#include <net/arp.h>
#include <net/ethernet.h>
#include <net/dhcp.h>
#include <net/network.h>
#include <keyboard.h>
#include <rtc.h>

#undef ENABLE_NETWORKING

kinfo kernel_info;
net_info networking_info;

Console *main_console;

extern void tcp_test( void );

extern "C" void kernel_main( void ) {
	// Begin with boostrap services
	serial_initalize();
	debugf( "Versions OS VI Debug Out\n" );
	load_limine_info();
	rtc_initalize();
	
	// Continue with core services, all of these need to boot in this order
	interrupt_initalize();
	syscall_initalize();
	acpi_initalize();
	timer_initalize();
	paging_initalize();
	memory_initalize();
	framebuffer_initalize();
	kernel_symbols_initalize();
	pci_initalize();
	task_initalize();
	keyboard_initalize();

	// Next setup the main console for use. From here on out, printf is okay.
	main_console = new Console( 0, 0, kernel_info.framebuffer_info.width, kernel_info.framebuffer_info.height );
	printf( "Versions OS VI\n" );
	
	// Service startup order from here onwards really shouldn't matter too much
	#ifdef ENABLE_NETWORKING
	memset( &networking_info, 0, sizeof( net_info ) );
	e1000_initalize();
	dhcp_start();
	#endif

	kshell_initalize();

	debugf( "Ending happy.\n" );
	printf( "Ending happy.\n" );
	do_immediate_shutdown();
}

extern "C" void do_test_send( void ) {
	//uint8_t dest[] = {127,0,0,2};
	//uint8_t dest[] = {192,168,12,1};
	uint8_t dest[] = {10,0,2,2};

	arp_send( (uint8_t *)&dest );
}

extern "C" void main_console_putc( char c ) {
	main_console->put_char( c );
}