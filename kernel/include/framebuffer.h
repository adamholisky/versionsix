#ifndef VIOS_FRAMEBUFFER_INCLUDED
#define VIOS_FRAMEBUFFER_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t * address;
	uint64_t pitch;
	uint64_t width;
	uint64_t height;
	uint16_t bpp;
	uint32_t pixel_width;
} framebuffer_information;

typedef struct {
    framebuffer_information *fb_info;
    uint32_t background_color;
    uint32_t foreground_color;
} framebuffer_state;

void framebuffer_initalize( void );
void fb_primative_fill_rect( uint8_t * buffer, uint32_t color, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
void fb_move_rect( uint8_t *buff, uint32_t dest_x, uint32_t dest_y, uint32_t dest_w, uint32_t dest_h, uint32_t src_x, uint32_t src_y, uint32_t src_w, uint32_t src_h );

#ifdef __cplusplus
}
#endif
#endif