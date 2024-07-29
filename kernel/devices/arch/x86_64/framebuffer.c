#include <kernel_common.h>
#include <framebuffer.h>

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
	uint32_t * where32 = (uint32_t *)where;
	unsigned int i, j;
	
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			*(where32 + j) = color;
		}
		where32 += fb_state.fb_info->pitch / 4;
	}
} 

void fb_move_rect( uint8_t *buff, uint32_t dest_x, uint32_t dest_y, uint32_t dest_w, uint32_t dest_h, uint32_t src_x, uint32_t src_y, uint32_t src_w, uint32_t src_h ) {
	unsigned int i = 0;
	uint8_t * mem_dest;
	uint8_t * mem_src;
	uint32_t *mem_dest32;
	uint32_t *mem_src32;
	unsigned int mem_size;


	for( i = 0; i < src_h; i++ ) {

		mem_dest32 = (uint32_t *)buff + dest_x + ((dest_y + i) * (fb_state.fb_info->pitch / 4));
		mem_src32 = (uint32_t *)buff + src_x + ((src_y + i) * (fb_state.fb_info->pitch / 4));
		mem_size = src_w;
		for(; mem_size != 0; mem_size--) *mem_dest32++ = *mem_src32++;
	}
}