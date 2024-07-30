#include <kernel_common.h>
#include <gui/gui.h>
#include <gui/console.h>
#include <gui/font.h>
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
	
	font_info *fi = font_get_main_info();
	con->char_width = fi->width;
	con->char_height = fi->height;

	con->num_cols = con->text_area_width / con->char_width;
	con->num_rows = con->text_area_height / con->char_height;

	con->fg_color = 0x00eaeaea;
	con->bg_color = 0x00232323;

	dfv( con->num_cols );
	dfv( con->num_rows );
	
	fb_primative_fill_rect( fb_state.fb_info->address, con->bg_color, con->pixel_top, con->pixel_left, con->pixel_width, con->pixel_height );
	
	con->buffer = (char *)kmalloc( sizeof(char) * con->num_cols * con->num_rows );
	memset( con->buffer, 0, sizeof(char) * con->num_cols * con->num_rows );
	vui_text_initalize( &con->text_area, con->text_area_top, con->text_area_left, con->text_area_width, con->text_area_height );

	con->current_row = 1;
	con->current_col = 1;
	con->tab_size = 4;

	con->show_cursor = false;
	con->blink_hidden = false;
}

void vui_console_put_char( vui_console *con, uint8_t c ) { 
	vui_console_put_char_at( con, c, con->current_row, con->current_col );
}

void vui_console_put_char_at( vui_console *con, uint8_t c, uint16_t row, uint16_t col ) {
	uint16_t cursor_x = con->current_pixel_x;
	uint16_t cursor_y = con->current_pixel_y;

	bool cursor_visibility = con->show_cursor;
	con->show_cursor = false;

	switch( c ) {
		case '\t':
			vui_console_do_tab( con );
			break;
		case '\n':
			vui_console_do_new_line( con );
			break;
		case '\b':
			vui_console_do_backspace( con );
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

	con->show_cursor = cursor_visibility;

	if( con->show_cursor == true ) {
		// Clear the previous cursor if a new line
		if( c == '\n' ) {
			vui_text_put_char_at_with_color( &con->text_area, ' ', cursor_x, cursor_y, con->fg_color, con->bg_color );
		}

		vui_console_update_cursor( con );
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

void vui_console_do_backspace( vui_console *con ) {
	bool cursor_visibility = con->show_cursor;

	// set to false while we're backspacing so a timer tick doesn't mess up the drawn cursor position
	con->show_cursor = false; 

	con->current_col--;
	con->current_pixel_x = con->current_pixel_x - con->char_width;
	vui_text_put_char_at_with_color( &con->text_area, ' ', con->current_pixel_x, con->current_pixel_y, con->fg_color, con->bg_color );

	if( cursor_visibility == true ) {
		vui_text_put_char_at_with_color( &con->text_area, ' ', con->current_pixel_x + con->char_width, con->current_pixel_y, con->fg_color, con->bg_color );
	}

	// restore cursor visiblity
	con->show_cursor = cursor_visibility;
}

/**
 * @brief Move the cursor its correct position.
 * 
 * @todo Redo this to keep track of the cursor position independently
 * 
 * @param con vui console to operate on
 */
void vui_console_update_cursor( vui_console *con ) {
	if( con->show_cursor == true ) {
		vui_text_put_char_at_with_color( &con->text_area, 0xDB, con->current_pixel_x, con->current_pixel_y, con->fg_color, con->bg_color );
	}
}

void vui_console_blink_cursor( vui_console *con ) {
	uint8_t c = 0;

	if( con->show_cursor == true ) {
		if( con->blink_hidden == true ) {
			c = 0xDB;
			con->blink_hidden = false;
		} else {
			c = ' ';
			con->blink_hidden = true;
		}

		vui_text_put_char_at_with_color( &con->text_area, c, con->current_pixel_x, con->current_pixel_y, con->fg_color, con->bg_color );
	}
}