#include <vui/vui.h>
#include <vui/console.h>
#include <vui/font.h>

/**
 * @brief 
 * 
 * @param x 
 * @param y 
 * @param width 
 * @param height 
 * @param parent 
 * @return vui_handle 
 */
vui_handle vui_console_create( uint16_t x, uint16_t y, uint16_t width, uint16_t height, vui_handle parent ) {
	vdf( "Create Console: x: %d    y: %d    width: %d    height: %d\n", x, y, width, height );

	vui_theme *theme = vui_get_active_theme();
	vui_console *con = vmalloc( sizeof(vui_console) );

	vui_handle H = vui_allocate_handle( VUI_HANDLE_TYPE_CONSOLE );
	vui_set_handle_data( H, con );

	con->handle = H;
	con->type = VUI_HANDLE_TYPE_CONSOLE;

	con->pixel_x= x;
	con->pixel_y = y;
	con->pixel_width = width;
	con->pixel_height = height;

	con->padding = 5;

	con->text_area_x = con->pixel_x + con->padding;
	con->text_area_y = con->pixel_y + con->padding;
	con->text_area_width = con->pixel_width - (2*con->padding);
	con->text_area_height = con->pixel_height - (2*con->padding);

	con->current_pixel_x = con->text_area_x;
	con->current_pixel_y = con->text_area_y;
	
	con->font = vui_font_get_font( "zap-vga" );
	con->char_width = con->font->info.width;
	con->char_height = con->font->info.height;

	vdf( "w: %d   h: %d\n", con->char_width, con->char_height );

	con->num_cols = (con->text_area_width / con->char_width);
	con->num_rows = con->text_area_height / con->char_height;

	con->fg_color = 0x00eaeaea;
	con->bg_color = 0x00232323;

	vdf( "    num_cols: %d    num_rows: %d\n", con->num_cols, con->num_rows );

	con->rows = vmalloc( sizeof(console_row) * con->num_rows );

	for( int i = 0; i < con->num_rows; i++ ) {
		con->rows[i].buff = vmalloc( sizeof(char) * con->num_cols );
		memset( con->rows[i].buff, 0, sizeof(char) * con->num_cols );

		con->rows[i].color_fg = vmalloc( sizeof(uint32_t) * con->num_cols );
		memset( con->rows[i].color_fg, 0, sizeof(uint32_t) * con->num_cols );

		con->rows[i].color_bg = vmalloc( sizeof(uint32_t) * con->num_cols );
		memset( con->rows[i].color_bg, 0, sizeof(uint32_t) * con->num_cols );

		con->rows[i].color_set_fg = vmalloc( sizeof(uint8_t) * con->num_cols );
		memset( con->rows[i].color_set_fg, 0, sizeof(uint8_t) * con->num_cols );

		con->rows[i].color_set_bg = vmalloc( sizeof(uint8_t) * con->num_cols );
		memset( con->rows[i].color_set_bg, 0, sizeof(uint8_t) * con->num_cols );

		con->rows[i].dirty = false;
	}

	con->current_row = 1;
	con->current_col = 1;
	con->tab_size = 4;

	con->show_cursor = false;
	con->blink_hidden = false;

	con->capturing_escape_code = false;
	con->capture_num = 0;
	con->use_color_override_bg = false;
	con->use_color_override_fg = false;
	memset( con->captured_escape, 0, 25 );

	vui_add_to_parent( parent, H );
	vui_create_cleanup(H);
	return H;
}

/**
 * @brief 
 * 
 * @param H 
 */
void vui_console_draw( vui_handle H ) {
	vui_console_draw_from_struct( vui_get_handle_data(H) );
}

/**
 * @brief 
 * 
 * @param con 
 */
void vui_console_draw_from_struct( vui_console *con ) {
	if( con->redraw_window ) {
		vui_draw_rect( con->pixel_x, con->pixel_y, con->pixel_width, con->pixel_height, con->bg_color );
	}
	
	for( int i = 0; i < con->num_rows; i++ ) {
		if( con->rows[i].dirty || con->redraw_window ) {
			//vdf( "dirty -- row: %d\n", i );
			uint16_t x = con->text_area_x;
			uint16_t y = con->text_area_y + (con->char_height * i);

			vui_draw_rect( x, y, con->text_area_width, con->char_height, con->bg_color );

			for( int j = 0; j < con->num_cols; j++ ) {
				char c = (con->rows[i].buff[j] == 0 ? ' ' : con->rows[i].buff[j]);

				uint32_t fg = con->rows[i].color_set_fg[j] != 0 ? con->rows[i].color_fg[j] : con->fg_color;
				uint32_t bg = con->rows[i].color_set_bg[j] != 0 ? con->rows[i].color_bg[j] : con->bg_color;

				//vdf( "draw fg: %08X\n", fg );

				if( con->rows[i].color_set_bg[j] != 0 ) {
					//vui_draw_rect( x, y, con->char_width, con->char_height, bg );
				}

				vui_draw_char( c, x, y, fg, bg, con->font, VUI_DRAW_FLAGS_NONE );

				x = x + con->char_width;
			}

			con->rows[i].dirty = false;
		}
	}

	con->redraw_window = false;
	vui_refresh_rect( con->pixel_x, con->pixel_y, con->pixel_width, con->pixel_height );
}

/**
 * @brief 
 * 
 * @param con 
 * @param c 
 */
void vui_console_put_char( vui_console *con, uint8_t c ) { 
	vui_console_put_char_at( con, c, con->current_row, con->current_col );
}

/**
 * @brief 
 * 
 * @param con 
 * @param c 
 * @param row 
 * @param col 
 */
void vui_console_put_char_at( vui_console *con, uint8_t c, uint16_t row, uint16_t col ) {
	uint16_t cursor_x = con->current_pixel_x;
	uint16_t cursor_y = con->current_pixel_y;

	bool cursor_visibility = con->show_cursor;
	con->show_cursor = false;

	//vdf( "new c: %x\n", c );

	switch( c ) {
		case '\x1b':
			con->capturing_escape_code = true;
			con->capture_num = 0;
			break;
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
			if( con->capturing_escape_code ) {
				vui_console_process_escape( con, c );
			} else {
				if( con->current_col == con->num_cols + 1 ) {
					vui_console_put_char( con, '\n' );
				}

				uint32_t fg = con->use_color_override_fg ? con->override_fg : con->fg_color;
				uint32_t bg = con->use_color_override_bg ? con->override_bg : con->bg_color;

				con->rows[row - 1].buff[col - 1] = c;
				con->rows[row - 1].color_fg[col - 1] = fg;
				con->rows[row - 1].color_bg[col - 1] = bg;
				con->rows[row - 1].color_set_fg[col - 1] = con->use_color_override_fg;;
				con->rows[row - 1].color_set_bg[col - 1] = con->use_color_override_bg;
				con->rows[row - 1].dirty = true;

				//vdf( "fg: 0x%08X\n", fg );

				if( con->rows[row - 1].color_set_bg[col - 1] != 0 ) {
					//vui_draw_rect( con->current_pixel_x, con->current_pixel_y, con->char_width, con->char_height, bg );
				}

				vui_draw_char( c, con->current_pixel_x, con->current_pixel_y, fg, bg, con->font, VUI_DRAW_FLAGS_IMMEDIATE );
				//vui_refresh_rect( con->current_pixel_x, con->current_pixel_y, con->char_width, con->char_height );

				con->current_col++;
				con->current_pixel_x = con->current_pixel_x + con->char_width;
			}			
	}

	con->show_cursor = cursor_visibility;

	if( con->show_cursor == true ) {
		// Clear the previous cursor if a new line
		if( c == '\n' ) {
			vui_draw_char( ' ', con->current_pixel_x, con->current_pixel_y, con->fg_color, con->bg_color, con->font, VUI_DRAW_FLAGS_NONE );
			//vui_refresh_rect( con->current_pixel_x, con->current_pixel_y, con->char_width, con->char_height );
		}

		vui_console_update_cursor( con );
	}
}

/**
 * @brief 
 * 
 * @param con 
 * @param c 
 */
void vui_console_process_escape( vui_console *con, char c ) {
	if( c == 'm' ) {
		con->captured_escape[ con->capture_num ] = 0;
		con->capturing_escape_code = false;
		
		int set_number = 0;
		char set_1[4];
		char set_2[4];
		char set_3[4];

		memset( set_1, 0, 4 );
		memset( set_2, 0, 4 );
		memset( set_3, 0, 4 );

		int n = 0;
		for( int i = 0; i < con->capture_num; i++ ) {
			if( con->captured_escape[i] == '[' ) {
				set_number = 1;
			} else {
				if( con->captured_escape[i] == ';' ) {
					set_number++;
					n = 0;
				} else {
					switch( set_number ) {
						case 1:
							set_1[n] = con->captured_escape[i];
							n++;
							break;
						case 2:
							set_2[n] = con->captured_escape[i];
							n++;
							break;
						case 3:
							set_3[n] = con->captured_escape[i];
							n++;
							break;
					}
				}
			}
		}

		int captured_fg = atoi(set_2);
		int captured_bg = atoi(set_3);

		//int captured_fg = ((con->captured_escape[3] - '0') * 10) + (con->captured_escape[4] - '0');
		//int captured_bg = ((con->captured_escape[6] - '0') * 10) + (con->captured_escape[7] - '0');

		uint32_t new_fg = 0;
		uint32_t new_bg = 0;

		switch( captured_fg ) {
			case 30:
				new_fg = CONSOLE_COLOR_RGB_BLACK;
				break;
			case 31:
				new_fg = CONSOLE_COLOR_RGB_RED;
				break;
			case 32:
				new_fg = CONSOLE_COLOR_RGB_GREEN;
				break;
			case 33:
				new_fg = CONSOLE_COLOR_RGB_BROWN;
				break;
			case 34:
				new_fg = CONSOLE_COLOR_RGB_BLUE;
				break;
			case 35:
				new_fg = CONSOLE_COLOR_RGB_MAGENTA;
				break;
			case 36:
				new_fg = CONSOLE_COLOR_RGB_CYAN;
				break;
			case 37:
				new_fg = CONSOLE_COLOR_RGB_LIGHT_GREY;
				break;
			case 90:
				new_fg = CONSOLE_COLOR_RGB_DARK_GREY;
				break;
			case 91:
				new_fg = CONSOLE_COLOR_RGB_LIGHT_RED;
				break;
			case 92:
				new_fg = CONSOLE_COLOR_RGB_LIGHT_GREEN;
				break;
			case 93:
				new_fg = CONSOLE_COLOR_RGB_YELLOW;
				break;
			case 94:
				new_fg = CONSOLE_COLOR_RGB_LIGHT_BLUE;
				break;
			case 95:
				new_fg = CONSOLE_COLOR_RGB_LIGHT_MAGENTA;
				break;
			case 96:
				new_fg = CONSOLE_COLOR_RGB_LIGHT_CYAN;
				break;
			case 97:
				new_fg = CONSOLE_COLOR_RGB_WHITE;
				break;
			default:
				new_fg = 0;
		}

		switch( captured_bg ) {
			case 40:
				new_bg = CONSOLE_COLOR_RGB_BLACK;
				break;
			case 41:
				new_bg = CONSOLE_COLOR_RGB_RED;
				break;
			case 42:
				new_bg = CONSOLE_COLOR_RGB_GREEN;
				break;
			case 43:
				new_bg = CONSOLE_COLOR_RGB_BROWN;
				break;
			case 44:
				new_bg = CONSOLE_COLOR_RGB_BLUE;
				break;
			case 45:
				new_bg = CONSOLE_COLOR_RGB_MAGENTA;
				break;
			case 46:
				new_bg = CONSOLE_COLOR_RGB_CYAN;
				break;
			case 47:
				new_bg = CONSOLE_COLOR_RGB_WHITE;
				break;
			case 100:
				new_fg = CONSOLE_COLOR_RGB_DARK_GREY;
				break;
			case 101:
				new_fg = CONSOLE_COLOR_RGB_LIGHT_RED;
				break;
			case 102:
				new_fg = CONSOLE_COLOR_RGB_LIGHT_GREEN;
				break;
			case 103:
				new_fg = CONSOLE_COLOR_RGB_YELLOW;
				break;
			case 104:
				new_fg = CONSOLE_COLOR_RGB_LIGHT_BLUE;
				break;
			case 105:
				new_fg = CONSOLE_COLOR_RGB_LIGHT_MAGENTA;
				break;
			case 106:
				new_fg = CONSOLE_COLOR_RGB_LIGHT_CYAN;
				break;
			case 107:
				new_fg = CONSOLE_COLOR_RGB_WHITE;
				break;
			default:
				new_bg = 0;
		}

		con->override_fg = new_fg;
		con->override_bg = new_bg;

		//vdf( "Cap fg: %d    Cap bg: %d\n", captured_fg, captured_bg );
		//vdf( "Overrides fg: 0x%08X    bg: 0x%08X\n", new_fg, new_bg );

		con->use_color_override_fg = new_fg == 0 ? false : true;
		con->use_color_override_bg = new_bg == 0 ? false : true;

		con->capture_num = 0;
	} else {
		//vdf( "added: %d -> %c\n", con->capture_num, c );
		con->captured_escape[ con->capture_num ] = c;
		con->capture_num++;
	}
}

/**
 * @brief
 * 
 * @param con 
 * @param foreground_color 
 * @param background_color 
 */
void vui_console_set_color( vui_console *con, uint32_t foreground_color, uint32_t background_color ) {
	con->fg_color = foreground_color;
	con->bg_color = background_color;
}

/**
 * @brief 
 * 
 * @param con 
 * @return uint32_t 
 */
uint32_t vui_console_get_foreground_color( vui_console *con ) {
	return con->fg_color;
}

/**
 * @brief 
 * 
 * @param con 
 * @return uint32_t 
 */
uint32_t vui_console_get_background_color( vui_console *con ) {
	return con->bg_color;
}

/**
 * @brief 
 * 
 * @param con 
 * @param str 
 */
void vui_console_put_string( vui_console *con, char *str ) {
	vui_console_put_string_at( con, str, con->current_row, con->current_col );
}

/**
 * @brief 
 * 
 * @param con 
 * @param str 
 * @param row 
 * @param col 
 */
void vui_console_put_string_at( vui_console *con, char *str, uint16_t row, uint16_t col ) {
	//vdf( "Starting put_string_at cur_row: %d   cur_col %d\n", con->current_row, con->current_col );

	for( ; *str; *str++ ) {
		vui_console_put_char_at( con, *str, con->current_row, con->current_col );
	}

	//vui_console_draw_from_struct( con );
}

/**
 * @brief 
 * 
 * @param con 
 * @param set_current_row_col 
 */
void vui_console_scroll_up( vui_console *con, bool set_current_row_col ) {
	for(int i = 0; i < con->num_rows - 1; i++ ) {
		memcpy( con->rows[i].buff, con->rows[i + 1].buff, sizeof(char) * con->num_cols );
		memcpy( con->rows[i].color_fg, con->rows[i + 1].color_fg, sizeof(uint32_t) * con->num_cols );
		memcpy( con->rows[i].color_bg, con->rows[i + 1].color_bg, sizeof(uint32_t) * con->num_cols );
		memcpy( con->rows[i].color_set_fg, con->rows[i + 1].color_set_fg, sizeof(uint8_t) * con->num_cols );
		memcpy( con->rows[i].color_set_bg, con->rows[i + 1].color_set_bg, sizeof(uint8_t) * con->num_cols );

		con->rows[i].dirty = false;
	}

	memset( con->rows[con->num_rows - 1].buff, 0, sizeof(char) * (con->num_cols) );
	memset( con->rows[con->num_rows - 1].color_set_fg, 0, sizeof(uint8_t) * (con->num_cols) );
	memset( con->rows[con->num_rows - 1].color_set_bg, 0, sizeof(uint8_t) * (con->num_cols) );
	memset( con->rows[con->num_rows - 1].color_fg, 0, sizeof(uint32_t) * (con->num_cols) );
	memset( con->rows[con->num_rows - 1].color_bg, 0, sizeof(uint32_t) * (con->num_cols) );


	// Move the console text up one line
	vui_move_rect(  con->text_area_x, con->text_area_y, 
					con->text_area_width, con->text_area_height - con->char_height,
					con->text_area_x, con->text_area_y + con->char_height, 
					con->text_area_width, con->text_area_height - con->char_height );
	
	// Fill in the last line to blank
	vui_draw_rect(  con->text_area_x, con->text_area_y + (con->num_rows - 1) * con->char_height, con->text_area_width, con->char_height, con->bg_color );

	// If we're asked to set row, col to the last line, do so
	if( set_current_row_col ) {
		con->current_pixel_x = con->text_area_x;
		con->current_pixel_y = con->text_area_y + ((con->num_rows - 1) * con->char_height);
		con->current_col = 1;
		con->current_row = con->num_rows;
	}

	vui_console_draw_from_struct( con );
}

/**
 * @brief 
 * 
 * @param con 
 */
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

/**
 * @brief 
 * 
 * @param con 
 */
void vui_console_do_new_line( vui_console *con ) {
	con->current_col = 1;
	
	if( con->current_row == con->num_rows ) {
		vui_console_scroll_up( con, true );
	} else {
		con->current_row++;
		con->current_pixel_y = con->current_pixel_y + con->char_height;
	}

	con->current_pixel_x = con->text_area_x;
}

/**
 * @brief 
 * 
 * @param con 
 */
void vui_console_do_backspace( vui_console *con ) {
	bool cursor_visibility = con->show_cursor;

	// set to false while we're backspacing so a timer tick doesn't mess up the drawn cursor position
	con->show_cursor = false; 

	con->current_col--;
	con->current_pixel_x = con->current_pixel_x - con->char_width;
	vui_draw_char( ' ', con->current_pixel_x, con->current_pixel_y, con->fg_color, con->bg_color, con->font, VUI_DRAW_FLAGS_NONE );
	vui_refresh_rect( con->current_pixel_x, con->current_pixel_y, con->char_width, con->char_height );

	if( cursor_visibility == true ) {
		vui_draw_char( ' ', con->current_pixel_x, con->current_pixel_y, con->fg_color, con->bg_color, con->font, VUI_DRAW_FLAGS_NONE );
		vui_refresh_rect( con->current_pixel_x, con->current_pixel_y, con->char_width, con->char_height );
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
		vui_draw_char( 0xDB, con->current_pixel_x, con->current_pixel_y, con->fg_color, con->bg_color, con->font, VUI_DRAW_FLAGS_NONE );
		vui_refresh_rect( con->current_pixel_x, con->current_pixel_y, con->char_width, con->char_height );
	}
}

/**
 * @brief 
 * 
 * @param con 
 */
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

		vui_draw_char( c, con->current_pixel_x, con->current_pixel_y, con->fg_color, con->bg_color, con->font, VUI_DRAW_FLAGS_NONE );
		vui_refresh_rect( con->current_pixel_x, con->current_pixel_y, con->char_width, con->char_height );
	}
}

/**
 * @brief 
 * 
 * @param H 
 */
void vui_console_tests( vui_handle H ) {
	char long_string_for_test[] = "\x1b[0;93;0mVersions VI\x1b[0;0;0m:\n";

	vui_console *con = vui_get_handle_data(H);

	vui_console_put_string( con, "VUI Console test suite.\n" );

	vui_console_put_string( con, long_string_for_test );

/* 	char str[50] = {};
	sprintf( str, "%c%c%c\n%c%c%c\n%c%c%c\n%c%c%c\n", 0xDA, 0xC4, 0xBf, 0x7C, 0xDB, 0x7C, 0x7C, 0xB2, 0x7C, 0xC0, 0xC4, 0xD9 );

	vui_console_put_string( con, str ); */

	/* vui_console_put_string( con, "\x1b[0;30;0mSo many escape code colors\n" );
	vui_console_put_string( con, "\x1b[0;31;0mSo many escape code colors\n" );
	vui_console_put_string( con, "\x1b[0;32;0mSo many escape code colors\n" );
	vui_console_put_string( con, "\x1b[0;33;0mSo many escape code colors\n" );
	vui_console_put_string( con, "\x1b[0;34;0mSo many escape code colors\n" );
	vui_console_put_string( con, "\x1b[0;35;0mSo many escape code colors\n" );
	vui_console_put_string( con, "\x1b[0;36;0mSo many escape code colors\n" );
	vui_console_put_string( con, "\x1b[0;37;0mSo many escape code colors\n" );
	vui_console_put_string( con, "\x1b[0;90;0mSo many escape code colors\n" );
	vui_console_put_string( con, "\x1b[0;91;0mSo many escape code colors\n" );
	vui_console_put_string( con, "\x1b[0;92;0mSo many escape code colors\n" );
	vui_console_put_string( con, "\x1b[0;93;0mSo many escape code colors\n" );
	vui_console_put_string( con, "\x1b[0;94;0mSo many escape code colors\n" );
	vui_console_put_string( con, "\x1b[0;95;0mSo many escape code colors\n" );
	vui_console_put_string( con, "\x1b[0;96;0mSo many escape code colors\n" );
	vui_console_put_string( con, "\x1b[0;97;0mSo many escape code colors\n" ); */

	
	con->redraw_window = true;
	vui_console_draw_from_struct( con );
}