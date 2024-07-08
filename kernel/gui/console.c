#include <kernel_common.h>
#include <gui/gui.h>
#include <gui/console.h>
#include <kmemory.h>
#include <framebuffer.h>

extern framebuffer_state fb_state;

void vui_console_initalize( vui_console *con, uint16_t top, uint16_t left, uint16_t width, uint16_t height ) {
	con->pixel_top = top;
	con->pixel_left = left;
	con->pixel_width = width;
	con->pixel_height = height;

	con->padding = 5;

	con->text_area_top = con->pixel_top + con->padding;
	con->text_area_left = con->pixel_left + con->padding;
	con->text_area_width = con->pixel_width - (2*con->padding);
	con->text_area_height = con->pixel_height - (2*con->padding);

	con->current_pixel_x = con->text_area_left;
	con->current_pixel_y = con->text_area_top;
	
	con->char_width = 8;
	con->char_height = 16;

	con->num_cols = con->text_area_width / con->char_width;
	con->num_rows = con->text_area_height / con->char_height;

	con->fg_color = COLOR_RGB_WHITE;
	con->bg_color = COLOR_RGB_BLUE;

	dfv( con->num_cols );
	dfv( con->num_rows );
	
	fb_primative_fill_rect( fb_state.fb_info->address, con->bg_color, con->pixel_top, con->pixel_left, con->pixel_width, con->pixel_height );
	
	con->buffer = (char *)kmalloc( sizeof(char) * con->num_cols * con->num_rows );
	memset( con->buffer, 0, sizeof(char) * con->num_cols * con->num_rows );
	vui_text_initalize( &con->text_area, con->text_area_top, con->text_area_left, con->text_area_width, con->text_area_height );

	con->current_row = 1;
	con->current_col = 1;
	con->tab_size = 4;
}

void vui_console_put_char( vui_console *con, char c ) { 
	vui_console_put_char_at( con, c, con->current_row, con->current_col );
}

void vui_console_put_char_at( vui_console *con, char c, uint16_t row, uint16_t col ) {
	switch( c ) {
		case '\t':
			vui_console_do_tab( con );
			break;
		case '\n':
			vui_console_do_new_line( con );
			break;
		default:
			if( con->current_col == con->num_cols + 1 ) {
				vui_console_put_char( con, '\n' );
			}

			*(con->buffer + (row*con->num_cols) + col) = c;
			vui_text_put_char_at_with_color( &con->text_area, c, con->current_pixel_x, con->current_pixel_y, con->fg_color, con->bg_color );

			con->current_col++;
			con->current_pixel_x = con->current_pixel_x + con->char_width;
	}
}

void vui_console_set_color( vui_console *con, uint32_t foreground_color, uint32_t background_color ) {
	con->fg_color = foreground_color;
	con->bg_color = background_color;
}

uint32_t vui_console_get_foreground_color( vui_console *con ) {
	return con->fg_color;
}

uint32_t vui_console_get_background_color( vui_console *con ) {
	return con->bg_color;
}

void vui_console_put_string( vui_console *con, char *str ) {
	vui_console_put_string_at( con, str, con->current_row, con->current_col );
}

void vui_console_put_string_at( vui_console *con, char *str, uint16_t row, uint16_t col ) {
	for( ; *str; *str++ ) {
		vui_console_put_char_at( con, *str, con->current_row, con->current_col );
	}
}

void vui_console_scroll_up( vui_console *con, bool set_current_row_col ) {
	// Move the console text up one line
	fb_move_rect( fb_state.fb_info->address,
							  con->text_area_left, con->text_area_top, con->text_area_width, con->text_area_height - con->char_height,
							  con->text_area_left, con->text_area_top + con->char_height, con->text_area_width, con->text_area_height - con->char_height );
	
	// Fill in the last line to blank
	fb_primative_fill_rect( fb_state.fb_info->address, con->bg_color, con->text_area_left, con->text_area_top + (con->num_rows - 1) * con->char_height, con->text_area_width, con->char_height );

	// If we're asked to set row, col to the last line, do so
	if( set_current_row_col ) {
		con->current_pixel_x = con->text_area_left;
		con->current_pixel_y = con->text_area_top + ((con->num_rows - 1) * con->char_height);
		con->current_col = 1;
		con->current_row = con->num_rows;
	}
}

void vui_console_do_tab( vui_console *con ) {
	// Only do a tab if there's space left. Need to look into this.
	if( con->current_col + con->tab_size < con->num_cols ) {
		if( con->current_col == 1 ) {
			for( int i = 0; i < con->tab_size; i++ ) {
				vui_console_put_char( con, ' ');
			}
		} else {
			int num_spaces = (con->current_col - 1) % con->tab_size;

			if( num_spaces == 0 ) {
				num_spaces = con->tab_size;
			}

			for( int i = 0; i < num_spaces; i++ ) {
				vui_console_put_char( con, ' ');
			}
		}
	}
}

void vui_console_do_new_line( vui_console *con ) {
	con->current_col = 1;
	
	if( con->current_row == con->num_rows ) {
		vui_console_scroll_up( con, true );
	} else {
		con->current_row++;
		con->current_pixel_y = con->current_pixel_y + con->char_height;
	}

	con->current_pixel_x = con->text_area_left;
}