#include "vui/vui.h"
#include "vui/window.h"

vui_handle vui_window_create( uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t flags ) {
	vdf( "Create Window: x: %d    y: %d    width: %d    height: %d\n", x, y, width, height );

	vui_theme *theme = vui_get_active_theme();
	vui_window *window = vmalloc( sizeof(vui_window) );
	memset( window, 0, sizeof(vui_window) );

	vui_handle H = vui_allocate_handle( VUI_HANDLE_TYPE_WINDOW );
	vui_set_handle_data( H, window );

	window->handle = H;
	window->type = VUI_HANDLE_TYPE_WINDOW;
	window->flags = flags;

	window->width = width;
	window->height = height;
	window->x = x;
	window->absolute_x = x;
	window->y = y;
	window->absolute_y = y;

	window->inner_width = width;
	window->inner_height = height;
	window->inner_x = x;
	window->inner_y = y;

	if( !(flags & VUI_WINDOW_FLAG_NO_BORDER) ) {
		window->width = width + VUI_WINDOW_BORDER_SIZE*2;
		window->height = height + VUI_WINDOW_BORDER_SIZE*2;
		window->inner_x = window->inner_x + VUI_WINDOW_BORDER_SIZE;
		window->inner_y = window->inner_y + VUI_WINDOW_BORDER_SIZE;
	}

	if( !(flags & VUI_WINDOW_FLAG_NO_TITLE_BAR) ) {
		window->height = window->height + VUI_WINDOW_TITLE_BAR_HEIGHT;
		window->inner_y = window->inner_y + VUI_WINDOW_TITLE_BAR_HEIGHT;
	}

	window->color_background = theme->window_background;

	memset( window->title, 0, VUI_WINDOW_TITLE_MAX );

	vdf( "    Final Window: x: %d    y: %d    width: %d    height: %d\n", window->x, window->y, window->width, window->height );
	vdf( "    Inner Window: x: %d    y: %d    width: %d    height: %d\n", window->inner_x, window->inner_y, window->inner_width, window->inner_height );

	vui_create_cleanup(H);
	return H;
}

void vui_window_draw( vui_handle H ) {
	vui_window_draw_from_struct( vui_get_handle_data(H) );
}

void vui_window_draw_from_struct( vui_window *window ) {
	vui_theme *theme = vui_get_active_theme();

	if( !(window->flags & VUI_WINDOW_FLAG_NO_BORDER) ) {
		// Border
		vui_draw_rect( window->x, window->y, window->width, window->height, theme->border );
	}

	if( !(window->flags & VUI_WINDOW_FLAG_NO_TITLE_BAR) ) {
		vui_draw_rect( window->inner_x, window->inner_y - VUI_WINDOW_TITLE_BAR_HEIGHT, window->inner_width, VUI_WINDOW_TITLE_BAR_HEIGHT, theme->window_title_bar_background );

		if( !(window->flags & VUI_WINDOW_FLAG_HIDE_TITLE) ) {
			vui_draw_string_ttf( window->title, window->inner_x + 3, window->inner_y - VUI_WINDOW_TITLE_BAR_HEIGHT + 1, theme->window_title_bar_foreground, theme->window_title_bar_background, vui_font_get_font("noto-sans-bold"), 13, VUI_DRAW_FLAGS_NONE );
		}
	}

	// Inner window
	vui_draw_rect( window->inner_x, window->inner_y, window->inner_width, window->inner_height, window->color_background );
}

/**
 * @brief Sets the background color of the window
 * 
 * @param H Handle to the window
 * @param color Color to set the window to
 */
void vui_window_set_background_color( vui_handle H, uint32_t color ) {
	vui_window *w = vui_get_handle_data(H);

	w->color_background = color;
}

/**
 * @brief Sets the window title
 * 
 * @param H Handle to the window
 * @param title String to set the title to
 */
void vui_window_set_title( vui_handle H, char *title ) {
	vui_window *w = vui_get_handle_data(H);

	strncpy( w->title, title, VUI_WINDOW_TITLE_MAX );
}