#ifndef VIOS_GUI_CONSOLE_INCLUDED
#define VIOS_GUI_CONSOLE_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <gui/text.h>

class Console {
	private:
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

		Text *text_area;
		char *buffer;
	public:
		void set_color( uint32_t foreground_color, uint32_t background_color );
		uint32_t get_foreground_color( void );
		uint32_t get_background_color( void );

		void put_char( char c );
		void put_char( char c, uint16_t row, uint16_t col );
		void put_string( char *str );
		void put_string( char *str, uint16_t row, uint16_t col );

		Console( uint16_t top, uint16_t left, uint16_t width, uint16_t height );
};

#ifdef __cplusplus
}
#endif
#endif