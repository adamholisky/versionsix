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
#include <net/arp.h>
#include <net/ethernet.h>
#include <net/dhcp.h>
#include <net/network.h>
#include <keyboard.h>
#include <rtc.h>
#include <ahci.h>
#include <fs.h>
#include <device.h>
#include <vui/vui.h>
#include <vui/console.h>
#include <vui/desktop.h>
#include <vui/font.h>
#include <vui/label.h>
#include <vui/window.h>
#include <vui/menubar.h>
#include <tests.h>
#include <sys_info.h>

#include <process.h>

#undef ENABLE_NETWORKING


#ifdef ENABLE_GUI
extern vui_core vui;
vui_handle main_console_handle;
vui_console *main_console;

void load_font_stuff( void );
void load_gui_stuff( void );
#endif

sys_info system_information;
kinfo kernel_info;
net_info networking_info;

extern void tcp_test( void );

char fxsave_region[512] __attribute__((aligned(16)));

void kernel_main( void ) {
	// Begin with boostrap services
	serial_initalize();
	debugf( "Versions OS VI Debug Out\n" );
	debugf( "Build Number: %d\n", BUILD_NUM );
	load_limine_info();
	rtc_initalize();
	acpi_initalize();
	interrupt_initalize();

	__asm__ volatile("cli");
	paging_setup_initial_structures();
	__asm__ volatile("sti");

	// Continue with core services, all of these need to boot in this order
	sse_initalize();
	syscall_initalize();
	timer_initalize();
	paging_initalize();
	memory_initalize();
	kernel_symbols_initalize();
	paging_initalize_page_groups();

	#ifdef ENABLE_PROFILING
	profiling_initalize();
	#endif

	#ifdef ENABLE_GUI
	framebuffer_initalize();
	#endif

	pci_initalize();
	ahci_initalize();

	// FS and device population, this needs to run in this order
	fs_initalize_part1();
	device_initalize();
	fs_initalize_part2();
	devices_populate_fs();

	klog( LOG_INFO, "Out of fs and dev setup" );

	task_initalize();
	keyboard_initalize();

	klog( LOG_INFO, "Out of task and kb setup" );

	system_information.version = 1;
	memcpy( &system_information.kernel_info, &kernel_info, sizeof(kinfo) );

	#ifdef ENABLE_GUI
	// Next setup the main console for use. From here on out, printf is okay.
	vui_init( (uint32_t *)kernel_info.framebuffer_info.address, 1024, 768 );

	// Forcing the font load b/c the FS is messed up. TODO: Remove
	for( int i = 0; i < (418820/0x1000) + 1; i++ ) {
		page_map( 0xFFFFFFFF40000000 + (0x1000 * i), 0x2800000 + (0x1000 * i) );
	}

	for( int i = 0; i < (10634/0x1000) + 1; i++ ) {
		page_map( 0xFFFFFFFF50000000 + (0x1000 * i), 0x2900000 + (0x1000 * i) );
	}

	load_font_stuff();
	load_gui_stuff();
	#endif
		
	
	printf( "Versions OS VI\n" );
	printf( "Build %d\n", BUILD_NUM );

	// Service startup order from here onwards really shouldn't matter too much
	#ifdef ENABLE_NETWORKING
	memset( &networking_info, 0, sizeof( net_info ) );
	e1000_initalize();	
	dhcp_start();
	#endif

	char test_message[] = "Test FS write to device?\n";
	int len = strlen( test_message );
	vfs_write( "/dev/stderr", test_message, 0, len );

	printf( "Initalizing process system and loading first exec...\n" );

	process_initalize();

	process_setup_init();

	kernel_idle_loop();
	
	do_immediate_shutdown();

	/* task_create( TASK_TYPE_KERNEL_THREAD, TASK_GENERATOR_DEV, "Task Chain", (uint64_t *)task_chain_a );

	tests_run_tests();

	task_create( TASK_TYPE_KERNEL_THREAD, TASK_GENERATOR_DEV, "KShell", (uint64_t *)kshell_initalize );
	syscall( SYSCALL_SCHED_YIELD, 0, NULL );

	// This is the "kernel idle task". We want to just check if someone has data ready, and if so, activate the task
	kernel_idle_loop();	

	debugf( "Ending happy.\n" );
	printf( "Ending happy.\n" );
	do_immediate_shutdown(); */
}

#ifdef ENABLE_GUI

void load_gui_stuff( void ) {
	vui_theme *theme = vui_get_active_theme();

	vui_handle menubar = vui_menubar_create();
	vui_handle_set_name( menubar, "main_menubar" );

	vui_handle desktop = vui_desktop_create( 0, 25, vui.width, vui.height - 25, VUI_DESKTOP_FLAG_NONE );
	vui_handle smooth_text = vui_label_create( 5, 768 - 25, "Versions OS 6.0.0.1", VUI_LABEL_FLAG_NONE, desktop );
	vui_label_set_color( smooth_text, COLOR_RGB_WHITE, theme->desktop );
	vui_handle_set_name( desktop, "desktop" );

	vui_handle win = vui_window_create( 25, 40, 500, 400, VUI_WINDOW_FLAG_NONE );
	vui_window_set_title( win, "ViOS 6" );
	vui_handle_set_name( win, "window_console" );
	vui_window *win_s = vui_get_handle_data(win);
	vui_window_set_background_color( win, 0x232323 );

	main_console_handle = vui_console_create( win_s->inner_x, win_s->inner_y, win_s->inner_width, win_s->inner_height, win );
	main_console = vui_get_handle_data( main_console_handle );
	vui_add_to_parent( win, main_console_handle );

	vui_draw( menubar );
	vui_draw( desktop );
	vui_draw( win );
}

void load_font_stuff( void ) {
	vui_font_initalize();
	//vui_font_load( VUI_FONT_TYPE_PSF, "zap-light", "/usr/share/zap-light20.psf" );
	vui_font_load( VUI_FONT_TYPE_PSF, "zap-vga", "/usr/share/zap-ext-vga16.psf" );
/* 	vui_font_load( VUI_FONT_TYPE_TTF, "dejavu-sans", "/usr/share/fonts/DejaVuSans.ttf" );
	vui_font_load( VUI_FONT_TYPE_TTF, "dejavu-sans-bold", "/usr/share/fonts/DejaVuSans-Bold.ttf" );
	vui_font_load( VUI_FONT_TYPE_TTF, "dejavu-sans-italic", "/usr/share/fonts/DejaVuSans-Oblique.ttf" ); */
	vui_font_load( VUI_FONT_TYPE_TTF, "noto-sans", "/usr/share/NotoSans-Regular.ttf" );
	vui_font_load( VUI_FONT_TYPE_TTF, "noto-sans-bold", "/usr/share/NotoSans-SemiBold.ttf" );
}

void main_console_putc( uint8_t c ) {
	vui_console_put_char( main_console, c );
	//main_console->redraw_window = true;
	//vui_console_draw_from_struct( main_console );
}

void main_console_set_cursor_visiblity( bool visible ) {
	main_console->show_cursor = visible;
}

void main_console_blink_cursor( void ) {
	//vui_console_blink_cursor( main_console );
}

#else
	void main_console_putc( uint8_t c ) {
		serial_write_port( c, COM3 );
	}

	void main_console_set_cursor_visiblity( bool visible ) {
		//main_console->show_cursor = visible;
	}

	void main_console_blink_cursor( void ) {
		//vui_console_blink_cursor( main_console );
	}
#endif

void task_chain_a( void ) {
	printf( "In A\n" );
	task_chain_b();
}

void task_chain_b( void ) {
	printf( "In B\n" );
	task_chain_c();
}

void task_chain_c( void ) {
	printf( "In C\n" );
	task_chain_d();
}

void task_chain_d( void ) {
	printf( "In D\n" );
	do {
		syscall( SYSCALL_SCHED_YIELD, 0, NULL );
	} while (1);
}