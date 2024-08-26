#include "vit.h"
#include "vui/vui.h"
#include "vui/event.h"
#include "vui/font.h"
#include "vui/desktop.h"
#include "vui/window.h"
#include "vui/label.h"
#include "vui/console.h"
#include "vui/button.h"
#include "vui/menubar.h"
#include "vui/schrift.h"

extern vui_core vui;

/**
 * @brief Draws the object given by the handle
 * 
 * @param H vui_handle handle
 */
void vui_draw_handle( vui_handle H ) {
	switch( vui.handles[H].type ) {
		case VUI_HANDLE_TYPE_WINDOW:
			vui_window_draw_from_struct( vui.handles[H].data );
			break;
		case VUI_HANDLE_TYPE_DESKTOP:
			vui_desktop_draw_from_struct( vui.handles[H].data );
			break;
		case VUI_HANDLE_TYPE_LABEL:
			vui_label_draw_from_struct( vui.handles[H].data );
			break;
		case VUI_HANDLE_TYPE_CONSOLE:
			vui_console_draw_from_struct( vui.handles[H].data );
			break;
		case VUI_HANDLE_TYPE_BUTTON:
			vui_button_draw_from_struct( vui.handles[H].data );
			break;
		case VUI_HANDLE_TYPE_MENUBAR:
			vui_menubar_draw_from_struct( vui.handles[H].data );
			break;
		default:
			vdf( "VUI: Cannot find handle type to draw.\n" );
	}

	vui_refresh();
}

/**
 * @brief Gets the data from the given handle
 * 
 * @param H Allocated handle
 * @return void* Pointer to the handle's data
 */
void *vui_get_handle_data( vui_handle H) {
	return vui.handles[H].data;
}

/**
 * @brief 
 * 
 * @return vui_theme* 
 */
vui_theme *vui_get_active_theme( void ) {
	return &vui.active_theme;
}

/**
 * @brief Refresh the entire VUI screen
 * 
 */
void vui_refresh( void ) {
	vdf( "Refreshed entire ui.\n" );

	memcpy( vui.fb, vui.buffer, 4 * vui.width * vui.height );

	#ifdef VI_ENV_DEV
	vit_gui_update();
	#endif
}

void vui_refresh_handle( vui_handle H ) {
	vui_common *vc = vui_get_handle_data(H);

	if( vc == NULL ) {
		vdf( "Refresh handle %d returned NULL. Aborting.\n", H );
		return;
	}

	vui_refresh_rect( vc->absolute_x, vc->absolute_y, vc->width, vc->height );
}

/**
 * @brief Refresh the given rect on the VUI screen
 * 
 * @param x x coords
 * @param y y coords
 * @param width width in pixels
 * @param height height in pixels
 */
void vui_refresh_rect( uint16_t x, uint16_t y, uint16_t width, uint16_t height ) {
	//vdf( "Refreshed: x: %d    y: %d    width: %d    height: %d\n", x, y, width, height );

	uint32_t offset_x = x;
	uint32_t offset_y = y * vui.pitch/4;

	for( int i = 0; i < height; i++ ) {
		for( int j = 0; j < width; j++ ) {
			*(vui.fb + offset_y + offset_x + j) = *(vui.buffer + offset_y + offset_x + j);
		}

		offset_y = offset_y + vui.pitch/4;
	}

	#ifdef VI_ENV_DEV
	vit_gui_update();
	#endif
}

/**
 * @brief Draw a recetangle on the screen
 * 
 * @param x 
 * @param y 
 * @param width 
 * @param height 
 * @param color 
 */
void vui_draw_rect( uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color ) {
	uint32_t offset_x = x;
	uint32_t offset_y = y * vui.pitch/4;

	for( int i = 0; i < height; i++ ) {
		for( int j = 0; j < width; j++ ) {
			*(vui.buffer + offset_y + offset_x + j) = color;
		}

		offset_y = offset_y + vui.pitch/4;
	}
}

/**
 * @brief Draws a character in the given color
 * 
 * @param char_num 
 * @param x 
 * @param y 
 * @param fg 
 * @param bg 
 */
void vui_draw_char( uint16_t char_num, uint16_t x, uint16_t y, uint32_t fg, uint32_t bg, vui_font *font, uint64_t flags ) {
	bool smoothing = ~vui.disable_font_smoothing;

 	uint8_t fg_red = (fg & 0x00FF0000) >> 16;
	uint8_t fg_green = (fg & 0x0000FF00) >> 8;
	uint8_t fg_blue = (fg & 0x000000FF);

	uint8_t bg_red = (bg & 0x00FF0000) >> 16;
	uint8_t bg_green = (bg & 0x0000FF00) >> 8;
	uint8_t bg_blue = (bg & 0x000000FF);

	uint8_t adjust_red = 0;
	uint8_t adjust_green = 0;
	uint8_t adjust_blue = 0;

	if( bg > fg ) {
		adjust_red = ((uint16_t)(fg_red) + (uint16_t)(bg_red))/10;
		adjust_green = ((uint16_t)(fg_green) + (uint16_t)(bg_green))/10;
		adjust_blue = ((uint16_t)(fg_blue) + (uint16_t)(bg_blue))/10;

		adjust_red = bg_red - adjust_red;
		adjust_green = bg_green - adjust_green;
		adjust_blue = bg_blue - adjust_blue;
	} else {
		adjust_red = ((uint16_t)(fg_red) + (uint16_t)(bg_red))/5;
		adjust_green = ((uint16_t)(fg_green) + (uint16_t)(bg_green))/5;
		adjust_blue = ((uint16_t)(fg_blue) + (uint16_t)(bg_blue))/5;

		adjust_red = bg_red + adjust_red;
		adjust_green = bg_green + adjust_green;
		adjust_blue = bg_blue + adjust_blue;
	}

	uint32_t smoothing_color = (adjust_red << 16) | (adjust_green << 8) | adjust_blue;

    //vdf( "fg: 0x%X, bg: 0x%X, smoothing color: 0x%08X\n", fg, bg, smoothing_color );

	int16_t index = -1;
	
	for( int n = 0; n < font->info.num_glyphs; n++ ) {
		if( font->bitmaps[n].num == char_num ) {
			index = n;
			n = font->info.num_glyphs;
		}
	}

	if( index == -1 ) {
		vdf( "Cannot find bitmap for glyph number 0x%04X (%d).\n", char_num, char_num );
	}

	uint16_t width_mod = 0;

	if( font->info.width <= 8 ) {
		width_mod = 1;
	}

	// If we have the immediate flag set, output right to the framebuffer
	

	// Draw the background -- only if we don't get the transparent flag
	if( !(flags & VUI_DRAW_FLAGS_TRANSPARENT) ) {
		for( int i = 0; i < font->info.height; i++ ) {
			uint32_t *loc = vui.buffer + ((y+i) * (vui.pitch / 4)) + x;

			for( int j = 0; j != font->info.width; j++ ) {
				*(loc + j) = bg;
			}
		}
	}

	//vdf( "printing: %c 0x%04X (%d).\n", font->bitmaps[index].num, font->bitmaps[index].num, font->bitmaps[index].num);
	for( int i = 0; i < font->info.height; i++ ) {
		uint32_t *loc = vui.buffer + ((y+i) * (vui.pitch / 4)) + x;
		uint32_t *loc_imm = vui.fb + ((y+i) * (vui.pitch / 4)) + x;
		
		//vdf( "Row: %d == %X\n", i, font->bitmaps[index].pixel_row[i] );
		//vdf( "\"" );
		for( int j = 0; j != font->info.width; j++ ) {
			if( ((font->bitmaps[index].pixel_row[i] >> j) & 0x1) ) {
				//vdf( "*" );
				*(loc + j) = fg;
				if( flags & VUI_DRAW_FLAGS_IMMEDIATE ) { *(loc_imm + j) = fg; }

				
			} else {
				//vdf( " " );
			}

			if( smoothing ){
				if( ((font->aa_mask[index].pixel_row[i] >> j) & 0x1) ) {
					//vdf( "*" );
					*(loc + j) = smoothing_color;
					if( flags & VUI_DRAW_FLAGS_IMMEDIATE ) { *(loc_imm + j) = smoothing_color; }

					
				}
			}
		}
		//vdf( "\"\n" );
	}

	#ifdef VI_ENV_DEV
	if( flags & VUI_DRAW_FLAGS_IMMEDIATE ) {
		vit_gui_update();
	}
	#endif
}

/**
 * @brief 
 * 
 * @param s 
 * @param x 
 * @param y 
 * @param fg 
 * @param bg 
 * @param font 
 * @param smoothing 
 */
void vui_draw_string( char *s, uint16_t x, uint16_t y, uint32_t fg, uint32_t bg, vui_font *font, uint64_t flags ) {
	int len = strlen(s);
	int current_x = x;
	vui_font *f = ( font == NULL ? vui_font_get_main_font() : font );

	for( int i = 0; i < len; i++ ) {
		if( f->type == VUI_FONT_TYPE_TTF ) {
			vui_draw_char_ttf( *s, current_x, y, fg, bg, f, flags );

			SFT_Glyph test_glyph;
			sft_lookup( &font->sft, *s, &test_glyph );
			SFT_GMetrics glyph_metrics;
			sft_gmetrics( &font->sft, test_glyph, &glyph_metrics );
			//font->info.width = glyph_metrics.advanceWidth;

			current_x = current_x + font->ttf_bitmaps->advance;
		} else {
			vui_draw_char( *s, current_x, y, fg, bg, f, flags );
			current_x = current_x + f->info.width;
		}
		
		s++;
	}
}

void vui_draw_string_ttf( char *s, uint16_t x, uint16_t y, uint32_t fg, uint32_t bg, vui_font *font, uint16_t size, uint64_t flags ) {
	int len = strlen(s);
	int current_x = x;
	vui_font *f = ( font == NULL ? vui_font_get_main_font() : font );

	for( int i = 0; i < len; i++ ) {
		if( f->type == VUI_FONT_TYPE_TTF ) {
			uint32_t char_num = *s;

			vui_draw_char_ttf( char_num, current_x, y, fg, bg, f, flags );

			//vdf( "final width: %d\n", font->ttf_bitmaps[char_num].advance + 1 );

			current_x = current_x + font->ttf_bitmaps[char_num].advance + 1;
		}
		
		s++;
	}
}

/**
 * @brief 
 * 
 * @param dest_x 
 * @param dest_y 
 * @param dest_w 
 * @param dest_h 
 * @param src_x 
 * @param src_y 
 * @param src_w 
 * @param src_h 
 */
void vui_move_rect( uint32_t dest_x, uint32_t dest_y, uint32_t dest_w, uint32_t dest_h, uint32_t src_x, uint32_t src_y, uint32_t src_w, uint32_t src_h ) {
	unsigned int i = 0;
	uint8_t * mem_dest;
	uint8_t * mem_src;
	uint32_t *mem_dest32;
	uint32_t *mem_src32;
	unsigned int mem_size;

	for( i = 0; i < src_h; i++ ) {
		mem_dest32 = vui.buffer + dest_x + ((dest_y + i) * (vui.pitch / 4));
		mem_src32 = vui.buffer + src_x + ((src_y + i) * (vui.pitch / 4));
		mem_size = src_w;
		for(; mem_size != 0; mem_size--) *mem_dest32++ = *mem_src32++;
	}
}

void vui_draw_char_ttf( uint32_t char_num, uint16_t x, uint16_t y, uint32_t fg, uint32_t bg, vui_font *font, uint64_t flags ) {
	//vdf( "draw: %X (%c)\n", char_num, (char)char_num );

	uint32_t y_offset = (font->size - font->ttf_bitmaps[(uint8_t)char_num].y_offset);

	for( int i = 0; i < font->ttf_bitmaps[(uint8_t)char_num].height + y_offset; i++ ) {
		uint32_t pix_row = i;
		if( y_offset != 0 && i >= y_offset) {
			pix_row = i - y_offset;
			//vdf( "%c: %d ->  %d ", (char)char_num, y_offset, pix_row );
		}

		uint32_t *loc = vui.buffer + ((y+i) * (vui.pitch / 4)) + x + font->ttf_bitmaps[char_num].x_offset;
		uint32_t *loc_imm = vui.fb + ((y+i) * (vui.pitch / 4)) + x + font->ttf_bitmaps[char_num].x_offset;
		
		//vdf( "Row: %d == %X\n", i, font->bitmaps[index].pixel_row[i] );
		//vdf( "\"" );
		//for( int j = 0; j != font->info.width; j++ ) {
		for( int j = 0; j != font->ttf_bitmaps[char_num].width; j++ ) {
			// handle the y-offset
			if( i < y_offset ) {
				//vdf( "offset %d (max: %d)\n", i, abs() );

				if( !(flags & VUI_DRAW_FLAGS_TRANSPARENT) ) {
					*(loc + j) = bg;
					if( flags & VUI_DRAW_FLAGS_IMMEDIATE ) { *(loc_imm + j) = bg; }
				}
				continue;
			}

			// render as normal
			if( font->ttf_bitmaps[(uint8_t)char_num].pixel[(pix_row * font->ttf_bitmaps[(uint8_t)char_num].width) + j] ) {
				//vdf( "*" );

				float adjust = font->ttf_bitmaps[(uint8_t)char_num].pixel[(pix_row * font->ttf_bitmaps[(uint8_t)char_num].width) + j];

				uint8_t red_bg = ((bg & 0x00FF0000) >> 16);
				uint8_t red = ((fg & 0x00FF0000) >> 16);
				red = (uint8_t)( (red * (adjust / 0xFF)) + (((1 - (adjust / 0xFF))) * red_bg));

				uint8_t green_bg = (bg & 0x0000FF00) >> 8;
				uint8_t green = (fg & 0x0000FF00) >> 8;
				green = (uint8_t)( (green * (adjust / 0xFF)) + (((1 - (adjust / 0xFF))) * green_bg));
				
				uint8_t blue_bg = (bg & 0x000000FF);
				uint8_t blue = (fg & 0x000000FF);
				blue = (uint8_t)( (blue * (adjust / 0xFF)) + (((1 - (adjust / 0xFF))) * blue_bg));

				uint32_t color = (red << 16) | (green << 8) | (blue);

				*(loc + j) = color;
				if( flags & VUI_DRAW_FLAGS_IMMEDIATE ) { *(loc_imm + j) = color; }

				
			} else {
				//vdf( " " );

				if( !(flags & VUI_DRAW_FLAGS_TRANSPARENT) ) {
					*(loc + j) = bg;
					if( flags & VUI_DRAW_FLAGS_IMMEDIATE ) { *(loc_imm + j) = bg; }
				}
			}
		}
		//vdf( "\"\n" );
	}
}