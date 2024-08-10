#include <vui/vui.h>
#include <vui/desktop.h>

vui_handle vui_desktop_create( uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t flags ) {
	vui_theme *theme = vui_get_active_theme();
	vui_handle H = vui_allocate_handle( VUI_HANDLE_TYPE_DESKTOP );

	vui_desktop *desktop = vmalloc( sizeof(vui_desktop) );
	memset( desktop, 0, sizeof(vui_desktop) );

	desktop->type = VUI_HANDLE_TYPE_DESKTOP;
	desktop->handle = H;
	desktop->flags = flags;
	desktop->priority = 0xFFFFFFFF;	// Desktop gets the lowest priority in the event chain

	desktop->x = x;
	desktop->y = y;
	desktop->width = width;
	desktop->height = height;

	desktop->color_background = theme->desktop;

	vui_set_handle_data( H, desktop );

	vui_create_cleanup(H);
	return H;
}

void vui_desktop_draw( vui_handle H ) {
	vui_desktop_draw_from_struct( vui_get_handle_data(H) );
}

void vui_desktop_draw_from_struct( vui_desktop *desktop ) {
	vui_draw_rect( desktop->x, desktop->y, desktop->width, desktop->height, desktop->color_background );
}

void vui_desktop_set_background_color( vui_handle H, uint32_t color ) {
	vui_desktop *d = vui_get_handle_data(H);
	d->color_background = color;
}