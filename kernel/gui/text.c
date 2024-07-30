#include <kernel_common.h>
#include <framebuffer.h>
#include <gui/gui.h>
#include <gui/text.h>
#include <gui/font.h>

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

void draw_char( uint16_t char_num, uint16_t x, uint16_t y ) {
	draw_char_with_color( char_num, x, y, COLOR_RGB_WHITE, COLOR_RGB_BLUE );
}

void draw_char_with_color( uint16_t char_num, uint16_t x, uint16_t y, uint32_t fg, uint32_t bg ) {
	font_bitmap *bitmaps = font_get_main_bitmap();
	font_info *info = font_get_main_info();

/* 	uint8_t red = (fg & 0x00FF0000) >> 16;
	uint8_t green = (fg & 0x0000FF00) >> 8;
	uint8_t blue = (fg & 0x000000FF);

	red = (((fg & 0x00FF0000) >> 16)/4);
	green = (((fg & 0x0000FF00) >> 8)/4);
	blue = ((fg & 0x000000FF)/4); */

	//uint32_t smoothing_color = (red << 16) | (green << 8) | blue;
	uint32_t smoothing_color = ((((fg & 0x00FF0000) >> 16)/4) << 16) | ((((fg & 0x0000FF00) >> 8)/4) << 8) | ((fg & 0x000000FF)/4);

	//smoothing_color = 0x00FF0000;

	//debugf( "smoothing color: 0x%08X\n", smoothing_color );

	int16_t index = -1;
	for( int n = 0; n < info->num_glyphs; n++ ) {
		if( bitmaps[n].num == char_num ) {
			index = n;
			n = info->num_glyphs;
		}
	}

	if( index == -1 ) {
		debugf( "Cannot find bitmap for glyph number 0x%04X (%d).\n", char_num, char_num );
	}

	//debugf( "printing: %c 0x%04X (%d).\n", bitmaps[index].num, bitmaps[index].num, bitmaps[index].num);
	for( int i = 0; i < info->height; i++ ) {
		uint32_t *loc = (uint32_t *)fb_state.fb_info->address + ((y+i) * (fb_state.fb_info->pitch / 4)) + x;

		//debugf( "Row: %d == %X\n", i, bitmaps[index].pixel_row[i] );
		//debugf_raw( "\"" );
		for( int j = 16; j != (16 - info->width); j-- ) {
			if( ((bitmaps[index].pixel_row[i] >> j) & 0x1) ) {
				//debugf_raw( "*" );
				*(loc + (16 - j)) = fg;

				/**
				 * X!
				 * !E
				 */
				if( (i + 1 <= info->height) && (j - 1 >= (16 - info->width)) ) { // 1 down and 1 right can happen
					//debugf( "1 Can happen.\n" );
					if( ((bitmaps[index].pixel_row[i + 1] >> (j-1)) & 0x1) ) { // if it exists
						if( !((bitmaps[index].pixel_row[i + 1] >> j) & 0x1) ) {  // if 1 down from current j does not exit
							//debugf( "AA apply!\n" );
							*(loc + (fb_state.fb_info->pitch / 4) + (16 - j)) = smoothing_color;// then fill it
						}

						/* if( !((bitmaps[index].pixel_row[i] >> (j-1)) & 0x1) ) {  // if 1 over from current j does not exit
							//debugf( "AA apply!\n" );
							*(loc + (16 - j - 1)) = smoothing_color;// then fill it
						} */
					}

				} 

				/**
				 * !X
				 * E!
				 */
				if( (i + 1 <= info->height) && (j + 1 >= (16 - info->width)) ) { // 1 down and 1 left can happen
					//debugf( "2 Can happen.\n" );
					if( ((bitmaps[index].pixel_row[i + 1] >> (j+1)) & 0x1) ) { // if it exists
						if( !((bitmaps[index].pixel_row[i + 1] >> j) & 0x1) ) {  // if 1 down from current j does not exit
							//debugf( "AA apply!\n" );
							*(loc + (fb_state.fb_info->pitch / 4) + (16 - j)) = smoothing_color;// then fill it
						}

						/* if( !((bitmaps[index].pixel_row[i] >> (j + 1)) & 0x1) ) {  // if 1 across from current j does not exit
							//debugf( "AA apply!\n" );
							*(loc + (16 - j + 1)) = smoothing_color;// then fill it
						} */
					}

				} 

			} else {
				//debugf_raw( " " );
				*(loc + (16 - j)) = bg;
			}
		}
		//debugf_raw( "\"\n" );
	}
}