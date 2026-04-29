#include <vui/vui.h>
#include <vui/desktop.h>
#include <spng.h>
#include <vfs.h>

#define BG_FILE "/share/img/mac-9-wallpaper-1024-768.png"

uint32_t* rendered_png;
extern vui_core vui;

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

	// Temp PNG background code
file_stats fstat;
	if ( vfs_getattr( BG_FILE, &fstat ) == VFS_ERROR_NONE ) {
		uint8_t* png_data = kmalloc( fstat.st_size );

		vfs_read( BG_FILE, png_data, fstat.st_size, 0 );

		spng_ctx* ctx = spng_ctx_new( 0 );

		spng_set_png_buffer( ctx, png_data, fstat.st_size );

		int format = SPNG_FMT_PNG;

		size_t out_size;
		spng_decoded_image_size( ctx, format, &out_size );

		rendered_png = kmalloc( out_size );
		spng_decode_image( ctx, rendered_png, out_size, format, 0 );

		kfree( png_data );
	}
	return H;
}

void vui_desktop_draw( vui_handle H ) {
	vui_desktop_draw_from_struct( vui_get_handle_data(H) );
}

void vui_desktop_draw_from_struct( vui_desktop *desktop ) {
	//vui_draw_rect( desktop->x, desktop->y, desktop->width, desktop->height, desktop->color_background );

	uint32_t offset_x = desktop->x;
	uint32_t offset_y = desktop->y * vui.pitch/4;

	for( int i = 0; i < desktop->height; i++ ) {
		for( int j = 0; j < desktop->width; j++ ) {
			uint32_t png_color = rendered_png[ offset_y + offset_x + j ];
			uint32_t os_color = (png_color&0x000000FF) << 16;
			os_color = os_color | ((png_color&0x0000FF00));
			os_color = os_color | (png_color&0x00FF0000) >> 16;

			*( vui.fb + offset_y + j ) = os_color;
			*( vui.buffer + offset_y + offset_x + j ) = os_color;
		}

		offset_y = offset_y + vui.pitch/4;
	}
}

void vui_desktop_set_background_color( vui_handle H, uint32_t color ) {
	vui_desktop *d = vui_get_handle_data(H);
	d->color_background = color;
}