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

void fb_move_rect( uint8_t *buff, uint32_t dest_x, uint32_t dest_y, uint32_t dest_w, uint32_t dest_h, uint32_t src_x, uint32_t src_y, uint32_t src_w, uint32_t src_h ) {
	unsigned int i = 0;
	uint8_t * mem_dest;
	uint8_t * mem_src;
	unsigned int mem_size;

	for( i = 0; i < src_h; i++ ) {
		mem_dest = buff + (dest_x * fb_state.fb_info->pixel_width) + ((dest_y + i) * fb_state.fb_info->pitch );
		mem_src = buff + (src_x * fb_state.fb_info->pixel_width) + ((src_y + i) * fb_state.fb_info->pitch );
		mem_size = (fb_state.fb_info->pixel_width * src_w);


		for(; mem_size != 0; mem_size--) *mem_dest++ = *mem_src++;
	}
}