#ifndef VIOS_GUI_CONSOLE_INCLUDED
#define VIOS_GUI_CONSOLE_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <vui/vui.h>
#include <vui/font.h>

#define CONSOLE_COLOR_BLACK 0
#define CONSOLE_COLOR_BLUE 1
#define CONSOLE_COLOR_GREEN 2
#define CONSOLE_COLOR_CYAN 3
#define CONSOLE_COLOR_RED 4
#define CONSOLE_COLOR_MAGENTA 5
#define CONSOLE_COLOR_BROWN 6
#define CONSOLE_COLOR_LIGHT_GREY 7
#define CONSOLE_COLOR_DARK_GREY 8
#define CONSOLE_COLOR_LIGHT_BLUE 9
#define CONSOLE_COLOR_LIGHT_GREEN 10
#define CONSOLE_COLOR_LIGHT_CYAN 11
#define CONSOLE_COLOR_LIGHT_RED 12
#define CONSOLE_COLOR_LIGHT_MAGENTA 13
#define CONSOLE_COLOR_YELLOW 14
#define CONSOLE_COLOR_WHITE 15

#define CONSOLE_COLOR_RGB_BLACK 0x001E1E1E
#define CONSOLE_COLOR_RGB_BLUE 0x000000CC
#define CONSOLE_COLOR_RGB_GREEN	0x0000CC00
#define CONSOLE_COLOR_RGB_CYAN 0x0000CCCC
#define CONSOLE_COLOR_RGB_RED 0x00CC0000
#define CONSOLE_COLOR_RGB_MAGENTA 0x00AA00AA
#define CONSOLE_COLOR_RGB_BROWN 0x00AA5500
#define CONSOLE_COLOR_RGB_LIGHT_GREY 0x00AAAAAA
#define CONSOLE_COLOR_RGB_DARK_GREY 0x00555555
#define CONSOLE_COLOR_RGB_LIGHT_BLUE 0x005555FF
#define CONSOLE_COLOR_RGB_LIGHT_GREEN 0x0055FF55
#define CONSOLE_COLOR_RGB_LIGHT_CYAN 0x0055FFFF
#define CONSOLE_COLOR_RGB_LIGHT_RED	0x00FF5555
#define CONSOLE_COLOR_RGB_LIGHT_MAGENTA 0x00FF55FF
#define CONSOLE_COLOR_RGB_YELLOW 0x00FFFF55
#define CONSOLE_COLOR_RGB_WHITE 0x00FFFFFF


typedef struct {
	char *buff;
	uint8_t *color_set_fg;
	uint8_t *color_set_bg;
	uint32_t *color_fg;
	uint32_t *color_bg;
	bool dirty;
} console_row;

typedef struct {
	INSERT_VUI_COMMON

	uint16_t pixel_x;
	uint16_t pixel_y;
	uint16_t pixel_width;
	uint16_t pixel_height;
	
	uint16_t padding;

	uint16_t text_area_x;
	uint16_t text_area_y;
	uint16_t text_area_width;
	uint16_t text_area_height;

	uint16_t char_width;
	uint16_t char_height;
	
	uint16_t num_cols;
	uint16_t num_rows;

	uint16_t current_row;
	uint16_t current_col;

	uint16_t current_pixel_x;
	uint16_t current_pixel_y;

	uint32_t fg_color;
	uint32_t bg_color;

	uint32_t override_fg;
	uint32_t override_bg;

	uint32_t tab_size;

	bool capturing_escape_code;
	int	capture_num;
	char captured_escape[25];
	bool use_color_override_fg;
	bool use_color_override_bg;

	bool show_cursor;
	bool blink_hidden;
	bool redraw_window;

	vui_font *font;

	console_row *rows;
} vui_console;

vui_handle vui_console_create( uint16_t x, uint16_t y, uint16_t width, uint16_t height, vui_handle parent );
void vui_console_draw( vui_handle H );
void vui_console_draw_from_struct( vui_console *con );
void vui_console_tests( vui_handle H );

void vui_console_put_char( vui_console *con, uint8_t c );
void vui_console_put_char_at( vui_console *con, uint8_t c, uint16_t row, uint16_t col );
void vui_console_process_escape( vui_console *con, char c );
void vui_console_set_color( vui_console *con, uint32_t foreground_color, uint32_t background_color );
uint32_t vui_console_get_foreground_color( vui_console *con );
uint32_t vui_console_get_background_color( vui_console *con );
void vui_console_put_string( vui_console *con, char *str );
void vui_console_put_string_at( vui_console *con, char *str, uint16_t row, uint16_t col );
void vui_console_scroll_up( vui_console *con, bool set_current_row_col );
void vui_console_do_tab( vui_console *con );
void vui_console_do_new_line( vui_console *con );
void vui_console_do_backspace( vui_console *con );
void vui_console_update_cursor( vui_console *con );
void vui_console_blink_cursor( vui_console *con );

#ifdef __cplusplus
}
#endif
#endif