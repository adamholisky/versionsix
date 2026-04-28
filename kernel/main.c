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
#include <ksymbols.h>
#include <pci.h>
#include <e1000.h>
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

#ifdef ENABLE_NETWORKING
void enable_networking();
#endif

#ifdef ENABLE_GUI
extern vui_core vui;
vui_handle main_console_handle;
vui_console *main_console;

void enable_gui( void );
void load_font_stuff( void );
void load_gui_stuff( void );
extern void tcp_test( void );
#endif

sys_info system_information;
kinfo kernel_info;
net_info networking_info;

extern void *kernel_stack;

char fxsave_region[512] __attribute__((aligned(16)));

void kernel_main( void ) {
	// Begin with boostrap services
	serial_initalize();
	debugf( "Versions OS VI Debug Out\n" );
	debugf( "Build Number: %d\n", BUILD_NUM );

	debugf( "Kernel stack top:                 0x%016llX\n", &kernel_stack );
	uint64_t rbp_value;
	__asm__ volatile ("mov %%rbp, %0" : "=r"(rbp_value));
	debugf( "Kernel rbp at early exec:         0x%016llX\n", rbp_value );
	

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

	pci_initalize();
	ahci_initalize();

	// FS and device population, this needs to run in this order
	fs_initalize_part1();
	device_initalize();
	fs_initalize_part2();
	devices_populate_fs();

	// GUI gets enabled here, or terminal redirects to.... ?
	#ifdef ENABLE_GUI
		enable_gui();
	#endif

	// Printf is now okay
	printf( "Versions OS VI\n" );
	printf( "Build %d\n", BUILD_NUM );

	keyboard_initalize();
	process_initalize();

	system_information.version = 1;
	memcpy( &system_information.kernel_info, &kernel_info, sizeof(kinfo) );

	#ifdef ENABLE_NETWORKING
	enable_networking();
	#endif

	
	

	extern uint64_t kernel_heap_virtual_memory_next;
	printf( "kmalloc() heap at now:    0x%016llX\n", kernel_heap_virtual_memory_next);

	extern avs_list *mount_points;
	printf( "mount_points:             0x%016llX\n", mount_points );
	vfs_mount_point *rootmp = mount_points->head->data;
	vfs_mount_point *rfsmp = mount_points->tail->data;
	printf( "root mp:                  0x%016llX\n", rootmp );
	printf( "rfs mp:                   0x%016llX\n", rfsmp );
	printf( "root mp fs:               0x%016llX\n", rootmp->fs );
	printf( "root mp sanity:           %s\n", rootmp->root );
	printf( "rfs mp sanity:            %s\n", rfsmp->root );
			
	printf( "Startup done. Handing to init process.\n\n" );

	process_setup_init();

	kernel_idle_loop();

	printf( "Idle loop has ended.\n" );
	debugf( "Idle loop has ended.\n" );
	
	do_immediate_shutdown();
}

#ifdef ENABLE_NETWORKING
void enable_networking( void ) {
	memset( &networking_info, 0, sizeof( net_info ) );
	e1000_initalize();	
	dhcp_start();
}
#endif

#ifdef ENABLE_GUI
void enable_gui( void ) {
	framebuffer_initalize();

	vui_init( (uint32_t *)kernel_info.framebuffer_info.address, 1024, 768 );

	load_font_stuff();
	load_gui_stuff();
}

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

	klog( LOG_INFO, "Drawing" );

	vui_draw( menubar );
	vui_draw( desktop );
	vui_draw( win );
}

void load_font_stuff( void ) {
	vui_font_initalize();
	//vui_font_load( VUI_FONT_TYPE_PSF, "zap-light", "/usr/share/zap-light20.psf" );
	vui_font_load( VUI_FONT_TYPE_PSF, "zap-vga", "/share/fonts/zap-ext-vga16.psf" );
/* 	vui_font_load( VUI_FONT_TYPE_TTF, "dejavu-sans", "/usr/share/fonts/DejaVuSans.ttf" );
	vui_font_load( VUI_FONT_TYPE_TTF, "dejavu-sans-bold", "/usr/share/fonts/DejaVuSans-Bold.ttf" );
	vui_font_load( VUI_FONT_TYPE_TTF, "dejavu-sans-italic", "/usr/share/fonts/DejaVuSans-Oblique.ttf" ); */
	vui_font_load( VUI_FONT_TYPE_TTF, "noto-sans", "/share/fonts/NotoSans-Regular.ttf" );
	vui_font_load( VUI_FONT_TYPE_TTF, "noto-sans-bold", "/share/fonts/NotoSans-SemiBold.ttf" );
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