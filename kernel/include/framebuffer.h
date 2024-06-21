#ifndef VIOS_FRAMEBUFFER_INCLUDED
#define VIOS_FRAMEBUFFER_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#define COLOR_RGB_WHITE 0x00FFFFFF
#define COLOR_RGB_BLACK 0x00000000
#define COLOR_RGB_BLUE 0x000000FF

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

#ifdef __cplusplus
}
#endif
#endif