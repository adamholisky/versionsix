#include "vit.h"
#include "vui/vui.h"
#include "vui/font.h"
#include "vui/desktop.h"
#include "vui/window.h"
#include "vui/label.h"

vui_core vui;

/**
 * @brief Test main loop for VUI development. Works on dev and OS.
 * 
 */
void vui_main_test_loop( void ) {
	vui_font_initalize();

	#ifdef VI_ENV_OS
	vui_font_load( VUI_FONT_TYPE_PSF, "Zap Light", "/usr/share/fonts/zap-light20.psf" );
	vui_font_load( VUI_FONT_TYPE_PSF, "Zap VGA", "/usr/share/fonts/zap-ext-vga16.psf" );
	#else
	vui_font_load( VUI_FONT_TYPE_PSF, "Zap Light", "zap-light20.psf" );
	vui_font_load( VUI_FONT_TYPE_PSF, "Zap VGA", "zap-ext-vga16.psf" );
	#endif

	vui_theme *theme = vui_get_active_theme();

	vui_handle desktop = vui_desktop_create( 0, 0, vui.width, vui.height, VUI_DESKTOP_FLAG_NONE );
	vui_draw_handle( desktop );

	vui_handle smooth_text = vui_label_create( 30, 100, "Adam Holisky Versions OS 6.0.0.1", VUI_LABEL_FLAG_NONE );

	vui_handle not_smooth_text = vui_label_create( 30, 130, "ABCDEFGHIJKLMNOPQRSTUVWXYZ `1234567890-= ~!@#$%^&*()_+ \"\'/\\.,<>[]{}", VUI_LABEL_FLAG_NO_SMOOTHING );

	vui_label_set_color( smooth_text, COLOR_RGB_BLACK, theme->window_background );
	vui_label_set_color( not_smooth_text, COLOR_RGB_BLACK, theme->window_background );

	vui_label_set_font( not_smooth_text, vui_font_get_font("Zap VGA") );

	vui_handle win = vui_window_create( 25, 25, 800, 400, VUI_WINDOW_FLAG_NONE );
	vui_window_set_title( win, "ViOS 6" );

	vui_draw_handle( win );
	vui_draw_handle( smooth_text );
	vui_draw_handle( not_smooth_text );
}

/**
 * @brief Initalize the VUI GUI system
 * 
 * @param fb_addr Address of the FB that outputs to the display
 * @param width Width of display 
 * @param height Height of display
 */
void vui_init( uint32_t *fb_addr, uint16_t width, uint16_t height ) {
	vui.fb = fb_addr;
	vui.width = width;
	vui.height = height;
	vui.pitch = width * 4;
	vui.buffer = vmalloc( 4 * width * height );
	vui.handle_next = 0;

	memset( vui.buffer, 0, 4 * width * height );

	vui.active_theme.desktop = 0x00374760;
	vui.active_theme.border = 0x747474;
	vui.active_theme.window_background = 0x00D6D6D6;
	vui.active_theme.window_title_bar_background = 0x00363636;
	vui.active_theme.window_title_bar_foreground = 0x00EAEAEA;

	#ifndef VI_ENV_OS
	vui_main_test_loop();
	#endif
}

vui_handle vui_allocate_handle( uint16_t type ) {
	if( vui.handle_next >= VUI_HANDLES_MAX ) {
		vdf( "VUI: Reached max handles!\n" );
		return 0;
	}

	vui.handles[vui.handle_next].data = NULL;
	vui.handles[vui.handle_next].type = type;
	vui.handles[vui.handle_next].H = vui.handle_next;

	vui.handle_next++;

	return vui.handle_next - 1;
}

/**
 * @brief Sets the handle's data
 * 
 * @param H Assigned handle
 * @param data Pointer to the data
 */
void vui_set_handle_data( vui_handle H, void *data ) {
	vui.handles[H].data = data;
}

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

vui_theme *vui_get_active_theme( void ) {
	return &vui.active_theme;
}


/**
 * @brief Refresh the entire VUI screen
 * 
 */
void vui_refresh( void ) {
	memcpy( vui.fb, vui.buffer, 4 * vui.width * vui.height );

	#ifndef VI_ENV_OS
	vit_gui_update();
	#endif
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
	uint32_t offset_x = x;
	uint32_t offset_y = y * vui.pitch/4;

	for( int i = 0; i < height; i++ ) {
		for( int j = 0; j < width; j++ ) {
			*(vui.fb + offset_y + offset_x + j) = *(vui.buffer + offset_y + offset_x + j);
		}

		offset_y = offset_y + vui.pitch/4;
	}

	#ifndef VI_ENV_OS
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
void vui_draw_char_with_color( uint16_t char_num, uint16_t x, uint16_t y, uint32_t fg, uint32_t bg, vui_font *font, bool smoothing ) {
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

	//vdf( "printing: %c 0x%04X (%d).\n", bitmaps[index].num, bitmaps[index].num, bitmaps[index].num);
	for( int i = 0; i < font->info.height; i++ ) {
		uint32_t *loc = vui.buffer + ((y+i) * (vui.pitch / 4)) + x;

		//vdf( "Row: %d == %X\n", i, bitmaps[index].pixel_row[i] );
		//vdf( "\"" );
		for( int j = 16; j != (16 - font->info.width - width_mod); j-- ) {
			if( ((font->bitmaps[index].pixel_row[i] >> j) & 0x1) ) {
				//vdf( "*" );
				*(loc + (16 - j)) = fg;

				if( smoothing ){
					/**
					 * X!
					 * !E
					 */
					if( (i + 1 <= font->info.height) && (j - 1 >= (16 - font->info.width - width_mod)) ) { // 1 down and 1 right can happen
						//debugf( "1 Can happen.\n" );
						if( ((font->bitmaps[index].pixel_row[i + 1] >> (j-1)) & 0x1) ) { // if it exists
							if( !((font->bitmaps[index].pixel_row[i + 1] >> j) & 0x1) ) {  // if 1 down from current j does not exit
								//vdf( "AA apply!\n" );
								*(loc + (vui.pitch / 4) + (16 - j)) = smoothing_color;// then fill it
							}

							if( !((font->bitmaps[index].pixel_row[i] >> (j-1)) & 0x1) ) {  // if 1 over from current j does not exit
								//debugf( "AA apply!\n" );
								*(loc + (16 - j + 1)) = smoothing_color;// then fill it
							}
						}

					}

					/**
					 * !X
					 * E!
					 */
					if( (i + 1 <= font->info.height) && (j + 1 >= (16 - font->info.width -  width_mod)) ) { // 1 down and 1 left can happen
						//vdf( "2 Can happen.\n" );
						if( ((font->bitmaps[index].pixel_row[i + 1] >> (j+1)) & 0x1) ) { // if it exists
							if( !((font->bitmaps[index].pixel_row[i + 1] >> j) & 0x1) ) {  // if 1 down from current j does not exit
								//vdf( "smoothing 2a\n" );
								*(loc + (vui.pitch / 4) + (16 - j)) = smoothing_color;// then fill it
							}

							if( !((font->bitmaps[index].pixel_row[i] >> (j + 1)) & 0x1) ) {  // if 1 across from current j does not exit
								//vdf( "smoothing 2b\n" );
								*(loc + (16 - j - 1)) = smoothing_color;// then fill it
							}
						}

					}
				}
			} else {
				//vdf( " " );
				//*(loc + (16 - j)) = bg;
			}
		}
		//vdf( "\"\n" );
	}
}

void vui_draw_string( char *s, uint16_t x, uint16_t y, uint32_t fg, uint32_t bg, vui_font *font, bool smoothing ) {
	int len = strlen(s);
	int current_x = x;
	vui_font *f = ( font == NULL ? vui_font_get_main_font() : font );

	vdf( "Smoothing: %d\n", smoothing );

	for( int i = 0; i < len; i++ ) {
		vui_draw_char_with_color( *s, current_x, y, fg, bg, f, smoothing );
		s++;
		current_x = current_x + f->info.width;
	}
}