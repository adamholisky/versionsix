#include <kernel_common.h>
#include <gui/gui.h>
#include <gui/console.h>
#include <kmemory.h>
#include <framebuffer.h>

extern framebuffer_state fb_state;

Console::Console( uint16_t top, uint16_t left, uint16_t width, uint16_t height ) {
	pixel_top = top;
	pixel_left = left;
	pixel_width = width;
	pixel_height = height;

	padding = 5;

	text_area_top = pixel_top + padding;
	text_area_left = pixel_left + padding;
	text_area_width = pixel_width - (2*padding);
	text_area_height = pixel_height - (2*padding);

	current_pixel_x = text_area_left;
	current_pixel_y = text_area_top;
	
	char_width = 8;
	char_height = 16;

	num_cols = text_area_width / char_width;
	num_rows = text_area_height / char_height;

	fg_color = COLOR_RGB_WHITE;
	bg_color = COLOR_RGB_BLUE;

	dfv( num_cols );
	dfv( num_rows );
	
	fb_primative_fill_rect( fb_state.fb_info->address, bg_color, pixel_top, pixel_left, pixel_width, pixel_height );
	
	buffer = (char *)kmalloc( sizeof(char) * num_cols * num_rows );
	memset( buffer, 0, sizeof(char) * num_cols * num_rows );
	text_area = new Text( text_area_top, text_area_left, text_area_width, text_area_height );
}

void Console::put_char( char c ) { 
	put_char( c, current_row, current_col );
}

void Console::put_char( char c, uint16_t row, uint16_t col ) {
	switch( c ) {
		case '\n':
			current_col = 0;
			
			if( current_row == num_rows ) {
				// TODO: scroll up

			} else {
				current_row++;
				current_pixel_y = current_pixel_y + char_height;
			}

			current_pixel_x = text_area_left;
			break;
		default:
			*(buffer + (row*num_cols) + col) = c;
			text_area->put_char( c, current_pixel_x, current_pixel_y, fg_color, bg_color );

			current_col++;
			current_pixel_x = current_pixel_x + char_width;

			if( current_col == num_cols ) {
				put_char( '\n' );
			}
	}
}

void Console::set_color( uint32_t foreground_color, uint32_t background_color ) {
	fg_color = foreground_color;
	bg_color = background_color;
}

uint32_t Console::get_foreground_color( void ) {
	return fg_color;
}

uint32_t Console::get_background_color( void ) {
	return bg_color;
}

void Console::put_string( char *str ) {
	put_string( str, current_row, current_col );
}

void Console::put_string( char *str, uint16_t row, uint16_t col ) {
	for( ; *str; *str++ ) {
		put_char( *str, current_row, current_col );
	}
}