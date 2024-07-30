#include <kernel_common.h>
#include <framebuffer.h>
#include <gui/gui.h>
#include <gui/text.h>
#include <fs.h>
#include <vfs.h>

extern framebuffer_state fb_state;

void vui_text_initalize( vui_text *txt, uint16_t top, uint16_t left, uint16_t width, uint16_t height ) {
	txt->pixel_top = top;
	txt->pixel_left = left;
	txt->pixel_width = width;
	txt->pixel_height = height;
}

void vui_text_put_char( vui_text *txt, uint8_t c ) {
	/* uint8_t temp_string[2] = {c, 0};
	ssfn_render( &ssfn_context, &ssfn_buffer, temp_string ); */
}

void vui_text_put_char_unicode( vui_text *txt, uint32_t c ) {
	/* uint8_t temp_string[2] = {c, 0};
	ssfn_render( &ssfn_context, &ssfn_buffer, temp_string ); */
}

void vui_text_put_char_at( vui_text *txt, uint8_t c, uint16_t x, uint16_t y ) {
	vui_text_put_char_at_unicode( txt, c, x, y );
}

void vui_text_put_char_at_unicode( vui_text *txt, uint32_t c, uint16_t x, uint16_t y )  {
	draw_char( c, x, y );
	/* uint8_t temp_string[2] = {c, 0};

	ssfn_buffer.x = x;
	ssfn_buffer.y = y;
	ssfn_render( &ssfn_context, &ssfn_buffer, temp_string ); */
}

void vui_text_put_char_at_with_color( vui_text *txt, uint8_t c, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color ) {
	vui_text_put_char_at_with_color_unicode( txt, (uint32_t)c, x, y, foreground_color, background_color );
}

void vui_text_put_char_at_with_color_unicode( vui_text *txt, uint32_t c, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color ) {
	draw_char_with_color( c, x, y, foreground_color, background_color );
}

void vui_text_put_string( vui_text *txt, char *str ) {
	//ssfn_render( &ssfn_context, &ssfn_buffer, str );
}

/**
 * Encode a code point using UTF-8
 * 
 * @author Ondřej Hruška <ondra@ondrovo.com>
 * @license MIT
 * 
 * @param out - output buffer (min 5 characters), will be 0-terminated
 * @param utf - code point 0-0x10FFFF
 * @return number of bytes on success, 0 on failure (also produces U+FFFD, which uses 3 bytes)
 */
int utf8_encode(uint8_t *out, uint32_t utf)
{
  if (utf <= 0x7F) {
    // Plain ASCII
    out[0] = (char) utf;
    out[1] = 0;
    return 1;
  }
  else if (utf <= 0x07FF) {
    // 2-byte unicode
    out[0] = (char) (((utf >> 6) & 0x1F) | 0xC0);
    out[1] = (char) (((utf >> 0) & 0x3F) | 0x80);
    out[2] = 0;
    return 2;
  }
  else if (utf <= 0xFFFF) {
    // 3-byte unicode
    out[0] = (char) (((utf >> 12) & 0x0F) | 0xE0);
    out[1] = (char) (((utf >>  6) & 0x3F) | 0x80);
    out[2] = (char) (((utf >>  0) & 0x3F) | 0x80);
    out[3] = 0;
    return 3;
  }
  else if (utf <= 0x10FFFF) {
    // 4-byte unicode
    out[0] = (char) (((utf >> 18) & 0x07) | 0xF0);
    out[1] = (char) (((utf >> 12) & 0x3F) | 0x80);
    out[2] = (char) (((utf >>  6) & 0x3F) | 0x80);
    out[3] = (char) (((utf >>  0) & 0x3F) | 0x80);
    out[4] = 0;
    return 4;
  }
  else { 
    // error - use replacement character
    out[0] = (char) 0xEF;  
    out[1] = (char) 0xBF;
    out[2] = (char) 0xBD;
    out[3] = 0;
    return 0;
  }
}

font_bitmap *bitmaps = NULL;

void load_font( void ) {
	char font_path[] = "/share/fonts/gomme10x20n.bdf";

	vfs_stat_data stats;

	int stat_error = vfs_stat( vfs_lookup_inode(font_path), &stats );
	if( stat_error != VFS_ERROR_NONE ) {
		debugf( "Error: %d\n", stat_error );
		return 1;
	}

	uint8_t *data = vfs_malloc( stats.size );
	int read_err = vfs_read( vfs_lookup_inode(font_path), data, stats.size, 0 );
	if( read_err < VFS_ERROR_NONE ) {
		debugf( "Error when reading: %d\n", read_err );
		return 1;
	}

	data[ stats.size ] = 0;

	bool keep_going = true;
	bool in_char = false;
	uint16_t current_char = 0;
	bool in_bitmap = false;
	int bitmap_line = 0;
	int j = 0;

	do {
		char line[250];
		
		memset( line, 0, 250 );

		for( int i = 0; i < 250; i++ ) {
			if( *data == '\n' ) {
				data++;
				i = 250;
			} else {
				line[i] = *data++;
			}
		}

		if( in_char ) {
			if( in_bitmap ) {
				if( strncmp(line, "ENDCHAR", sizeof("ENDCHAR") - 1) == 0 ) {
					in_char = false;
					in_bitmap = false;
					bitmap_line = 0;
					j++;
				} else {
					bitmaps[j].pixel_row[bitmap_line] = strtol(line, NULL, 16);
					/* if( current_char == 'V' ) {
						debugf( "bitmap line: %X %X %X\n", bitmaps[j].pixel_row[bitmap_line], line_data, line_data16 );
					} */
					bitmap_line++;
				}
			} else {
				if( strncmp(line, "ENCODING ", sizeof("ENCODING ") - 1) == 0 ) {
					char *encoding_line = line;
					encoding_line = encoding_line + sizeof("ENCODING ") - 1;
					current_char = atoi(encoding_line);
					bitmaps[j].num = current_char;
					//debugf( "Found 0x%X (%d) '%c'\n", current_char, current_char, current_char );
				} else if( strncmp(line, "BITMAP", sizeof("BITMAP") - 1) == 0 ) {
					in_bitmap = true;
				}
			}
		} else {
			if( strncmp(line, "CHARS ", 6) == 0 ) {
				char *chars_line = line;
				chars_line = chars_line + 6;
				uint16_t num_chars = atoi(chars_line);
				bitmaps = kmalloc( sizeof(font_bitmap) * num_chars );
				//debugf( "Num Chars: %d\n", num_chars );
			} else if( strncmp(line, "STARTCHAR ", sizeof("STARTCHAR ") - 1) == 0 ) {
				in_char = true;
			}
		}

		if( *data == 0 ) {
			keep_going = false;
		}
	} while( keep_going );

	debugf( "added %d chars\n", j );
}

void draw_char( uint16_t char_num, uint16_t x, uint16_t y ) {
	draw_char_with_color( char_num, x, y, COLOR_RGB_WHITE, COLOR_RGB_BLUE );
}

void draw_char_with_color( uint16_t char_num, uint16_t x, uint16_t y, uint32_t fg, uint32_t bg ) {

	int16_t index = -1;
	for( int n = 0; n < 255; n++ ) {
		if( bitmaps[n].num == char_num ) {
			index = n;
			n = 255;
		}
	}

	if( index == -1 ) {
		debugf( "Cannot find bitmap for glyph number 0x%04X (%d).\n", char_num, char_num );
	}

	//debugf( "printing: %c 0x%04X (%d).\n", bitmaps[index].num, bitmaps[index].num, bitmaps[index].num);
	for( int i = 0; i < 20; i++ ) {
		uint32_t *loc = (uint32_t *)fb_state.fb_info->address + ((y+i) * (fb_state.fb_info->pitch / 4)) + x;

		//debugf( "Row: %d == %X\n", i, bitmaps[index].pixel_row[i] );
		//debugf_raw( "\"" );
		for( int j = 16; j != 6; j-- ) {
			if( ((bitmaps[index].pixel_row[i] >> j) & 0x1) ) {
				//debugf_raw( "*" );
				*(loc + (16 - j)) = fg;
			} else {
				//debugf_raw( " " );
			}
		}
		//debugf_raw( "\"\n" );
	}
}