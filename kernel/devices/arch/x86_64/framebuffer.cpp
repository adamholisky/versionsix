#include <kernel_common.h>
#include <framebuffer.h>
#include <gui/gui.h>
#include <gui/text.h>

extern kinfo kernel_info;
framebuffer_state fb_state;

#define DEBUG_FRAMEBUFFER_INITALIZE
void framebuffer_initalize( void ) {
    #ifdef DEBUG_FRAMEBUFFER_INITALIZE
    dfv( kernel_info.framebuffer_info.address );
    dfv( kernel_info.framebuffer_info.height );
    dfv( kernel_info.framebuffer_info.width );
    dfv( kernel_info.framebuffer_info.bpp );
    dfv( kernel_info.framebuffer_info.pitch );
    #endif

    fb_state.fb_info = &kernel_info.framebuffer_info;
    fb_state.background_color = 0x00000000;
    fb_state.foreground_color = 0x00000000;
}

void fb_primative_fill_rect( uint8_t * buffer, uint32_t color, unsigned int x, unsigned int y, unsigned int w, unsigned int h) {
	uint8_t * where = (buffer + (x * fb_state.fb_info->pixel_width) + (y * fb_state.fb_info->pitch ));
	unsigned int i, j;
	
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			*(uint32_t *)(where + (j * fb_state.fb_info->pixel_width) ) = color;
		}
		where += fb_state.fb_info->pitch;
	}
} 