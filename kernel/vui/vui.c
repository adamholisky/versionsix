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

vui_core vui;

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
	vui.handle_next = 1;

	memset( vui.buffer, 0, 4 * width * height );

	vui.active_theme.desktop = 0x00374760;
	vui.active_theme.border = 0x747474;
	vui.active_theme.window_background = 0x00D6D6D6;
	vui.active_theme.window_title_bar_background = 0x00363636;
	vui.active_theme.window_title_bar_foreground = 0x00EAEAEA;
	vui.active_theme.button_foreground = 0xD6D6D6;
	vui.active_theme.button_background = 0x0056C1;	
	vui.active_theme.button_hover = 0x0064E1;
	vui.active_theme.button_active = 0x003476;
	vui.active_theme.menubar_background = 0xD9D9D9;
	vui.active_theme.menubar_foreground = 0x363636;
}

/**
 * @brief 
 * 
 * @param type 
 * @return vui_handle 
 */
vui_handle vui_allocate_handle( uint16_t type ) {
	if( vui.handle_next >= VUI_HANDLES_MAX ) {
		vdf( "VUI: Reached max handles!\n" );
		return 0;
	}

	vui.handles[vui.handle_next].data = NULL;
	vui.handles[vui.handle_next].type = type;
	vui.handles[vui.handle_next].H = vui.handle_next;

	// If the handle that's being added is a dispatcher, then add it to the list
	if( vui_is_dispatcher(type) ) {	
		vui_handle_list_add( &vui.dispatchers, vui.handle_next );
	}

	vui.handle_next++;

	return vui.handle_next - 1;
}



/**
 * @brief 
 * 
 * @param H 
 */
void vui_create_cleanup( vui_handle H ) {
	vui_common *vc = vui_get_handle_data(H);

	if( vc == NULL ) {
		vdf( "vui_common is NULL. Big problem. Aborting.\n" );
		return;
	}

	if( vui_is_dispatcher(vc->type) ) {
		vui_sort_list_by_priority( &vui.dispatchers );
	}
}

/**
 * @brief 
 * 
 * @param H 
 * @return uint16_t 
 */
uint16_t vui_get_type_from_master_list( vui_handle H ) {
	return vui.handles[H].type;
}

/**
 * @brief Sets the internal name of the element
 * 
 * @param H 
 * @param name 
 */
void vui_handle_set_name( vui_handle H, char *name ) {
	vui_common *vc = vui_get_handle_data(H);

	strcpy(vc->name, name);
}

/**
 * @brief Returns the name of the handle
 * 
 * @param H 
 * @return char* 
 */
char *vui_handle_get_name( vui_handle H ) {
	vui_common *vc = vui_get_handle_data(H);

	return vc->name;
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
 * @brief Draws the given handle and all its children
 * 
 * @param H vui_handle of the element to draw
 */
void vui_draw( vui_handle H ) {
	// Step 1: Draw the container
	vui_draw_handle(H);

	// Step 2: Iterate through children, draw each
	vui_common *parent_st = vui_get_handle_data(H);

	if( parent_st == NULL ) {
		vdf( "Parent struct is null. Aborting.\n" );
		return;
	}

	vui_handle_list *top = &parent_st->children;

	do {
		if( top->H != 0 ) {
			vui_draw_handle( top->H );
		}
		
		top = top->next;
	} while( top != NULL );
}

/**
 * @brief Adds the given handle to the provided handle list
 * 
 * @param list Pointer to the list to add to
 * @param handle_to_add Handle to add 
 */
bool vui_handle_list_add( vui_handle_list *list, vui_handle handle_to_add ) {
	vui_handle_list *top = list;
	vui_handle_list *node = NULL;
	bool free_node_found = false;

	if( top == NULL ) {
		vdf( "Top is NULL. Aborting.\n" );
		return false;
	}

	do {
		if( top->next == NULL ) {
			if( top->H == 0 ) {
				node = top;
			} else {
				top->next = vmalloc( sizeof(vui_handle_list) );
				node = top->next;
			}

			free_node_found = true;
		} else {
			top = top->next;
		}
	} while( top != NULL && !free_node_found );

	if( node == NULL ) {
		vdf( "Free node returned as null. Aborting.\n" );
		return false;
	}

	node->H = handle_to_add;
	node->next = NULL;

	return true;
}

/**
 * @brief Adds the child handle to the parent
 * 
 * @param parent 
 * @param child 
 */
void vui_add_to_parent( vui_handle parent, vui_handle child ) {
	// Step 1: Get the data structs as vui_common
	vui_common *parent_st = vui_get_handle_data(parent);
	vui_common *child_st = vui_get_handle_data(child);

	if( parent_st == NULL ) {
		vdf( "Parent_st is NULL. Aborting.\n" );
		return;
	}

	if( child_st == NULL ) {
		vdf( "Child_st is NULL. Aborting.\n" );
		return;
	}

	// Step 2: Find the end of the list, add to it
	vui_handle_list *top = &parent_st->children;
	vui_handle_list_add( &parent_st->children, child_st->handle );
	child_st->parent = parent_st->handle;

	// Step 4: Sort parent's child handles by priority order
	// TODO ^^
}

/**
 * @brief Sorts the provided list by priority
 * 
 * @param list 
 */
void vui_sort_list_by_priority( vui_handle_list *list ) {
	bool keep_going = true;
	bool did_sort = false;

	//vdf( "Priority List Pre Sort:\n" );
	for( vui_handle_list *hl = list; hl != NULL; hl = hl->next ) {
		vui_common *vc = vui_get_handle_data(hl->H);
		//vdf( "   %d -> %08X\n", hl->H, vc->priority );
	}

	if( list->H == 0 ) {
		return;
	}

	if( list->next == NULL ) {
		return;
	}

	do {
		vui_handle_list *top = list;
		vui_handle_list *current = list->next;
		keep_going = true;
		did_sort = false;

		while( keep_going ) {
			vui_common *top_st = vui_get_handle_data(top->H);
			vui_common *current_st = vui_get_handle_data(current->H);
			vui_handle temp = 0;

			if( current_st->priority < top_st->priority ) {
				temp = top->H;
				top->H = current->H;
				current->H = temp;

				did_sort = true;
			}

			if( current->next == NULL ) {
				keep_going = false;
			} else {
				top = current;
				current = top->next;
			}
		}
	} while( did_sort );

	//vdf( "Priority List Post Sort:\n" );
	for( vui_handle_list *hl = list; hl != NULL; hl = hl->next ) {
		vui_common *vc = vui_get_handle_data(hl->H);
		//vdf( "   %d -> %08X\n", hl->H, vc->priority );
	}
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

	#ifndef VI_ENV_OS
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
void vui_draw_string( char *s, uint16_t x, uint16_t y, uint32_t fg, uint32_t bg, vui_font *font, bool smoothing ) {
	int len = strlen(s);
	int current_x = x;
	vui_font *f = ( font == NULL ? vui_font_get_main_font() : font );

	//vdf( "Smoothing: %d\n", smoothing );

	for( int i = 0; i < len; i++ ) {
		vui_draw_char_with_color( *s, current_x, y, fg, bg, f, smoothing );
		s++;
		current_x = current_x + f->info.width;
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