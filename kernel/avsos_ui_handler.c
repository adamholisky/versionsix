#include <kernel_common.h>
#include <vui/vui.h>
#include <vui/console.h>
#include <vui/desktop.h>
#include <vui/font.h>
#include <vui/label.h>
#include <vui/window.h>
#include <vui/menubar.h>
#include <avsos_ui.h>

extern vui_core vui;
vui_handle main_console_handle;
vui_console* main_console;

extern kinfo kernel_info;

void enable_gui( void ) {
	framebuffer_initalize();

	vui_init( (uint32_t*)kernel_info.framebuffer_info.address, 1024, 768 );

	load_font_stuff();
	//load_gui_stuff();
}

void load_gui_stuff( void ) {
	vui_theme* theme = vui_get_active_theme();

	vui_handle menubar = vui_menubar_create();
	vui_handle_set_name( menubar, "main_menubar" );

	vui_handle desktop = vui_desktop_create( 0, 25, vui.width, vui.height - 25, VUI_DESKTOP_FLAG_NONE );

	char desktop_string[50];
	memset( desktop_string, 0, 50 );
	sprintf( desktop_string, "avsOS build %d", BUILD_NUM );
	vui_handle smooth_text = vui_label_create( 5, 768 - 25, desktop_string, VUI_LABEL_FLAG_NONE, desktop );
	vui_label_set_color( smooth_text, COLOR_RGB_WHITE, theme->desktop );
	vui_handle_set_name( desktop, "desktop" );

	vui_handle win = vui_window_create( 25, 40, 500, 400, VUI_WINDOW_FLAG_NONE );
	vui_window_set_title( win, "avsOS Shell" );
	vui_handle_set_name( win, "window_console" );
	vui_window* win_s = vui_get_handle_data( win );
	vui_window_set_background_color( win, 0x232323 );

	main_console_handle = vui_console_create( win_s->inner_x, win_s->inner_y, win_s->inner_width, win_s->inner_height, win );
	main_console = vui_get_handle_data( main_console_handle );
	vui_add_to_parent( win, main_console_handle );

	klog( LOG_INFO, "Drawing" );

	vui_draw( menubar );
	vui_draw( desktop );
	vui_draw( win );

	vui_mouse_save_area(0,0);
	vui_draw_mouse_at(0,0);
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