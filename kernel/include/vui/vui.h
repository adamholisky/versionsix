#ifndef VIOS_VUI_INCLUDED
#define VIOS_VUI_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include "vit.h"
#include "vui/font.h"

#define VUI_HANDLES_MAX 512

#define COLOR_RGB_BLACK 0x00000000
#define COLOR_RGB_WHITE 0x00FFFFFF
#define COLOR_RGB_BLUE  0x000000FF
#define COLOR_RGB_GREEN 0x0000FF00
#define COLOR_RGB_RED   0x00FF0000

#define VUI_HANDLE_TYPE_WINDOW 1
#define VUI_HANDLE_TYPE_LABEL 2
#define VUI_HANDLE_TYPE_BUTTON 3
#define VUI_HANDLE_TYPE_DESKTOP 4

typedef uint32_t vui_handle;

typedef struct {
	vui_handle H;
	uint16_t type;
	void *data;
} vui_handle_list;

typedef struct {
	uint32_t desktop;
	uint32_t border;
	uint32_t window_background;
	uint32_t window_title_bar_background;
	uint32_t window_title_bar_foreground;
} vui_theme;

typedef struct {
	uint32_t *fb;
	uint32_t *buffer;
	uint16_t width;
	uint16_t height;
	uint16_t pitch;

	vui_theme active_theme;

	vui_handle_list handles[VUI_HANDLES_MAX];
	vui_handle handle_next;
} vui_core;

/**************************************/
/* Core Functions                     */
/**************************************/
void vui_init( uint32_t *fb_addr, uint16_t width, uint16_t height );
void vui_refresh( void );
void vui_refresh_rect( uint16_t x, uint16_t y, uint16_t width, uint16_t height );

vui_handle vui_allocate_handle( uint16_t type );
void vui_set_handle_data( vui_handle H, void *data );
void *vui_get_handle_data( vui_handle H);
void vui_draw_handle( vui_handle H );

vui_theme *vui_get_active_theme( void );


/**************************************/
/* Test Functions                     */
/**************************************/
void vui_main_test_loop( void );

/**************************************/
/* Drawing Primatives                 */
/**************************************/
void vui_draw_rect( uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color );
void vui_draw_string( char *s, uint16_t x, uint16_t y, uint32_t fg, uint32_t bg, vui_font *font, bool smoothing );
void vui_draw_char_with_color( uint16_t char_num, uint16_t x, uint16_t y, uint32_t fg, uint32_t bg, vui_font *font, bool smoothing );

#ifdef __cplusplus
}
#endif
#endif