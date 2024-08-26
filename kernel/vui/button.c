#include "vui/vui.h"
#include "vui/button.h"
#include "vui/event.h"

extern vui_core vui;

vui_handle vui_button_create( char *text, uint16_t x, uint16_t y, uint16_t width, uint16_t height, vui_handle parent ) {
	vdf( "Create Button: x: %d    y: %d    width: %d    height: %d\n", x, y, width, height );

	vui_theme *theme = vui_get_active_theme();
	vui_button *button = vmalloc( sizeof(vui_button) );
	memset( button, 0, sizeof(vui_button) );

	vui_handle H = vui_allocate_handle( VUI_HANDLE_TYPE_BUTTON );
	vui_set_handle_data( H, button );

	button->handle = H;
	button->type = VUI_HANDLE_TYPE_BUTTON;
	button->flags = 0;

	button->width = width;
	button->height = height;
	button->x = x;
	button->absolute_x = x;
	button->y = y;
	button->absolute_y = y;

	button->inner_width = width;
	button->inner_height = height;
	button->inner_x = x;
	button->inner_y = y;

	button->width = width + VUI_BUTTON_BORDER_SIZE*2;
	button->height = height + VUI_BUTTON_BORDER_SIZE*2;
	button->inner_x = button->inner_x + VUI_BUTTON_BORDER_SIZE;
	button->inner_y = button->inner_y + VUI_BUTTON_BORDER_SIZE;

	button->color_foreground = theme->button_foreground;
	button->color_background = theme->button_background;

	button->ops.default_on_mouse_down = vui_button_on_mouse_down;
	button->ops.default_on_mouse_up = vui_button_on_mouse_up;
	button->ops.default_on_mouse_enter = vui_button_on_mouse_enter;
	button->ops.default_on_mouse_exit = vui_button_on_mouse_exit;
	button->ops.default_on_mouse_move = vui_button_on_mouse_move;

	strcpy( button->text, text );

	vdf( "    Inner Button: x: %d    y: %d    width: %d    height: %d\n", button->inner_x, button->inner_y, button->inner_width, button->inner_height );

	vui_add_to_parent( parent, H );
	vui_create_cleanup(H);
	return H;
}

void vui_button_draw( vui_handle H ) {
	vui_button_draw_from_struct( vui_get_handle_data(H) );
}

void vui_button_draw_from_struct( vui_button *button ) {
	vui_theme *theme = vui_get_active_theme();

	uint32_t bg_color = button->color_background;

	if( button->is_hover ) {
		bg_color = theme->button_hover;

		if( button->is_active ) {
			bg_color = theme->button_active;
		}
	}

	// Border
	vui_draw_rect( button->x, button->y, button->width, button->height, theme->border );

	// Inner window
	vui_draw_rect( button->inner_x, button->inner_y, button->inner_width, button->inner_height, bg_color );

	// Text
	uint16_t width;
	uint16_t height;

	vui_font_get_bounding_box( button->text, vui_font_get_font("zap-vga"), &width, &height );

	uint16_t x = button->absolute_x + (button->width / 2) - (width / 2);
	uint16_t y = button->absolute_y + (button->height / 2) - (height / 2);

	vui_draw_string_ttf( button->text, x, y, button->color_foreground, bg_color, vui_font_get_font("noto-sans"), 13, VUI_DRAW_FLAGS_NONE );
}

/**
 * @brief Sets the background color of the window
 * 
 * @param H Handle to the window
 * @param color Color to set the window to
 */
void vui_window_set_color( vui_handle H, uint32_t color_foreground, uint32_t color_background ) {
	vui_button *b = vui_get_handle_data(H);

	b->color_foreground = color_foreground;
	b->color_background = color_background;
}

/**
 * @brief Sets the window title
 * 
 * @param H Handle to the window
 * @param title String to set the title to
 */
void vui_button_set_text( vui_handle H, char *text ) {
	vui_button *b = vui_get_handle_data(H);

	strncpy( b->text, text, VUI_BUTTON_TEXT_MAX );
}

void vui_button_on_mouse_move( vui_event *e ) {
	/* vui_button *b = vui_get_handle_data(e->H);

	if( !b->is_hover ) {
		vui_button_on_mouse_enter(e);
	} */
}

void vui_button_on_mouse_enter( vui_event *e ) {
	vui_button *b = vui_get_handle_data(e->H);

	b->is_hover = true;
	//vui.last_hover = e->H;

	//e->type = VUI_EVENT_MOUSE_ENTER;

	vui_button_draw_from_struct(b);
	vui_refresh_handle(e->H);
}

void vui_button_on_mouse_exit( vui_event *e ) {
	vui_button *b = vui_get_handle_data(e->H);

	b->is_hover = false;
	b->is_active = false; // to handle if moust is down and moved out of area

	//e->type = VUI_EVENT_MOUSE_EXIT;

	vui_button_draw_from_struct(b);
	vui_refresh_handle(e->H);
}

void vui_button_on_mouse_down( vui_event *e ) {
	vui_button *b = vui_get_handle_data(e->H);

	b->is_active = true;

	if( b->ops.on_mouse_down != NULL ) {
		b->ops.on_mouse_down(e);
	}

	vui_button_draw_from_struct(b);
	vui_refresh_handle(e->H);
}

void vui_button_on_mouse_up( vui_event *e ) {
	vui_button *b = vui_get_handle_data(e->H);

	b->is_active = false;

	if( b->ops.on_mouse_up != NULL ) {
		b->ops.on_mouse_up(e);
	}

	vui_button_draw_from_struct(b);
	vui_refresh_handle(e->H);
}