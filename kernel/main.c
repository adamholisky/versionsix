#include <kernel_common.h>
#include <limine_bootstrap.h>
#include <limine.h>
#include <devices/serial.h>
#include <devices/stdout.h>
#include <devices/stdin.h>
#include <acpi.h>
#include <file.h>
#include <interrupt.h>
#include <timer.h>
#include <page.h>
#include <kmemory.h>
#include <ksymbols.h>
#include <pci.h>
#include <elf.h>
#include <rtc.h>
#include <ahci.h>
#include <fs.h>
#include <device.h>
#include <sys_info.h>
#include <process.h>
#include <avs_dev_api.h>
#include <lib.h>
#include <wait_queue.h>
#include <avsos_ui.h>
#include <avsos_networking.h>
#include <profiling.h>

#define AVSLOADER_USE_GUI 0x1
#define AVSLOADER_USE_NETWORKING 0x2
#define AVSLOADER_USE_PROFILING 0x4

sys_info system_information;
kinfo kernel_info;

bool use_gui;
bool use_networking;
bool use_profiling;

extern void* kernel_stack;

char fxsave_region[512] __attribute__( ( aligned( 16 ) ) );

void kernel_main( void ) {
	// Begin with boostrap services
	serial_initalize();
	debugf( "avsOS Debug Out\n" );
	debugf( "Build Number: %d\n", BUILD_NUM );
	avs_dev_api_send_hello();

	load_limine_info();
	rtc_initalize();
	acpi_initalize();
	interrupt_initalize();
	serial_enable_interrupts();

	__asm__ volatile( "cli" );
	paging_setup_initial_structures();
	__asm__ volatile( "sti" );

	// Continue with core services, all of these need to boot in this order
	sse_initalize();
	syscall_initalize();
	timer_initalize();
	paging_initalize();
	memory_initalize();
	kernel_symbols_initalize();
	paging_initalize_page_groups();

	page_map( 0x1337C0DE, 0x1337C0DE );
	uint32_t *avsos_loader_config = 0x1337C0DE;

	if( *avsos_loader_config & AVSLOADER_USE_GUI ) {
		use_gui = true;
	} else {
		use_gui = false;
	}

	if( *avsos_loader_config & AVSLOADER_USE_NETWORKING ) {
		//use_networking = true;
	} else {
		use_networking = false;
	}

	if( *avsos_loader_config & AVSLOADER_USE_PROFILING ) {
		//use_profiling = true;
	} else {
		use_profiling = false;
	}

	if( use_profiling ) {
		profiling_initalize();
	}
	
	pci_initalize();
	ahci_initalize();

	// FS and device population, this needs to run in this order
	fs_initalize_part1();
	device_initalize();
	fs_initalize_part2();
	devices_populate_fs();

	lib_initalize();

	// GUI gets enabled here, or terminal redirects to.... ?
	if( use_gui ) {
		enable_gui();
	} else {
		device *d_com3 = device_get_major_minor_device( "serial", "3" );
		stdout_set_redirect( d_com3 );
		stdin_set_redirect( d_com3 );
	}

	system_information.version = 1;
	memcpy( &system_information.kernel_info, &kernel_info, sizeof( kinfo ) );

	process_initalize();
	process_setup_init();
	wq_initalize();

	// Libraries need manual loading, for now
	int register_result = lib_register( "/libc.so" );

	// Printf is now okay
	printf( "avsOS\n" );
	printf( "Build %d\n", BUILD_NUM );

	if( use_networking ) {
		enable_networking();
	}

	printf( "Startup done. Handing to init process.\n\n" );

	kernel_idle_loop();

	printf( "Idle loop has ended.\n" );
	debugf( "Idle loop has ended.\n" );

	do_immediate_shutdown();
}