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
#include <vui/vui.h>
#include <vui/console.h>
#include <net/arp.h>
#include <net/ethernet.h>
#include <net/dhcp.h>
#include <net/network.h>
#include <keyboard.h>
#include <rtc.h>
#include <ahci.h>
#include <fs.h>
#include <vui/font.h>
#include <device.h>

#undef ENABLE_NETWORKING

kinfo kernel_info;
net_info networking_info;

vui_console main_console;

extern void tcp_test( void );

char fxsave_region[512] __attribute__((aligned(16)));

void kernel_main( void ) {
	// Begin with boostrap services
	serial_initalize();
	debugf( "Versions OS VI Debug Out\n" );
	load_limine_info();
	rtc_initalize();
	
	// Continue with core services, all of these need to boot in this order
	interrupt_initalize();
	sse_initalize();
	syscall_initalize();
	acpi_initalize();
	timer_initalize();
	paging_initalize();
	memory_initalize();
	kernel_symbols_initalize();

	#ifdef VIOS_ENABLE_PROFILING
	profiling_initalize();
	#endif
	framebuffer_initalize();
	pci_initalize();
	ahci_initalize();

	// FS and device population, this needs to run in this order
	fs_initalize_part1();
	device_initalize();
	fs_initalize_part2();
	devices_populate_fs();

	task_initalize();
	keyboard_initalize();

	// Next setup the main console for use. From here on out, printf is okay.
	vui_init( (uint32_t *)kernel_info.framebuffer_info.address, 1024, 768 );
	vui_font_load( VUI_FONT_TYPE_PSF, "Zap Light", "/usr/share/fonts/zap-light20.psf" );
	vui_font_load( VUI_FONT_TYPE_PSF, "Zap VGA", "/usr/share/fonts/zap-ext-vga16.psf" );
	vui_console_initalize( &main_console, 0, 0, kernel_info.framebuffer_info.width, kernel_info.framebuffer_info.height );
	printf( "Versions OS VI\n" );

	// Service startup order from here onwards really shouldn't matter too much
	#ifdef ENABLE_NETWORKING
	memset( &networking_info, 0, sizeof( net_info ) );
	e1000_initalize();	
	dhcp_start();
	#endif

	char test_message[] = "Test FS write to device?\n";
	int len = strlen( test_message );
	vfs_write( vfs_lookup_inode("/dev/stderr0"), test_message, len, 0 );

	task_create( TASK_TYPE_KERNEL_THREAD, "Task Chain", (uint64_t *)task_chain_a );
	task_create( TASK_TYPE_KERNEL_THREAD, "KShell", (uint64_t *)kshell_initalize );
	syscall( SYSCALL_SCHED_YIELD, 0, NULL );

	// This is the "kernel idle task". We want to just check if someone has data ready, and if so, activate the task
	kernel_idle_loop();	

	debugf( "Ending happy.\n" );
	printf( "Ending happy.\n" );
	do_immediate_shutdown();
}

void main_console_putc( uint8_t c ) {
	vui_console_put_char( &main_console, c );
}

void main_console_set_cursor_visiblity( bool visible ) {
	main_console.show_cursor = visible;
}

void main_console_blink_cursor( void ) {
	vui_console_blink_cursor( &main_console );
}

void task_chain_a( void ) {
	task_chain_b();
}

void task_chain_b( void ) {
	task_chain_c();
}

void task_chain_c( void ) {
	task_chain_d();
}

void task_chain_d( void ) {
	do {
		syscall( SYSCALL_SCHED_YIELD, 0, NULL );
	} while (1);
}