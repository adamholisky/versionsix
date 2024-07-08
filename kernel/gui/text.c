#include <kernel_common.h>
#include <framebuffer.h>
#include <gui/gui.h>
#include <gui/text.h>

extern framebuffer_state fb_state;
extern char *u_vga16;

#define SSFN_CONSOLEBITMAP_TRUECOLOR        /* use the special renderer for 32 bit truecolor packed pixels */
#include <gui/ssfn.h>

void vui_text_initalize( vui_text *txt, uint16_t top, uint16_t left, uint16_t width, uint16_t height ) {
	txt->pixel_top = top;
	txt->pixel_left = left;
	txt->pixel_width = width;
	txt->pixel_height = height;

	ssfn_src = (ssfn_font_t *)&u_vga16;      /* the bitmap font to use */

	ssfn_dst.ptr = (uint8_t *)fb_state.fb_info->address;                  /* address of the linear frame buffer */
	ssfn_dst.w = width;                          /* width */
	ssfn_dst.h = height;                           /* height */
	ssfn_dst.p = fb_state.fb_info->pitch;                          /* bytes per line */
	ssfn_dst.x = left;
	ssfn_dst.y = top;                /* pen position */
	ssfn_dst.fg = 0x00FF0000;                     /* foreground color */
}

void vui_text_put_char( vui_text *txt, char c ) {
	ssfn_putc( c );
}

void vui_text_put_char_at( vui_text *txt, char c, uint16_t x, uint16_t y ) {
	ssfn_dst.x = x;
	ssfn_dst.y = y;
	ssfn_putc( c );
}

void vui_text_put_char_at_with_color( vui_text *txt, char c, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color ) {
	ssfn_dst.fg = foreground_color;
	ssfn_dst.bg = background_color;
	ssfn_dst.x = x;
	ssfn_dst.y = y;
	ssfn_putc( c );
}

void vui_text_put_string( vui_text *txt, char *str ) {
	for( ; *str; *str++ ) {
		ssfn_putc( *str );
	}
}