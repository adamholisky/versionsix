#ifndef VIOS_GUI_CONSOLE_INCLUDED
#define VIOS_GUI_CONSOLE_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <gui/text.h>

typedef struct {
	uint16_t pixel_top;
	uint16_t pixel_left;
	uint16_t pixel_width;
	uint16_t pixel_height;
	
	uint16_t padding;

	uint16_t text_area_top;
	uint16_t text_area_left;
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

	uint32_t tab_size;

	vui_text text_area;

	bool show_cursor;
	bool blink_hidden;

	char *buffer;
} vui_console;

void vui_console_initalize( vui_console *con, uint16_t top, uint16_t left, uint16_t width, uint16_t height );
void vui_console_put_char( vui_console *con, char c );
void vui_console_put_char_at( vui_console *con, char c, uint16_t row, uint16_t col );
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