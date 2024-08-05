#ifndef VIOS_VUI_WINDOW_INCLUDED
#define VIOS_VUI_WINDOW_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include "vui/vui.h"

#define VUI_WINDOW_FLAG_NONE				0
#define VUI_WINDOW_FLAG_NO_BORDER			(1 << 0)
#define VUI_WINDOW_FLAG_NO_TITLE_BAR		(1 << 1)
#define VUI_WINDOW_FLAG_HIDE_TITLE			(1 << 2)

#define VUI_WINDOW_BORDER_SIZE 1
#define VUI_WINDOW_TITLE_BAR_HEIGHT 20

#define VUI_WINDOW_TITLE_MAX 50

typedef struct {
	uint16_t handle;
	uint32_t flags;

	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;

	uint16_t inner_x;
	uint16_t inner_y;
	uint16_t inner_width;
	uint16_t inner_height;

	uint32_t color_background;

	char title[VUI_WINDOW_TITLE_MAX];


} vui_window;

vui_handle vui_window_create( uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t flags );
void vui_window_draw( vui_handle H );
void vui_window_draw_from_struct( vui_window *window );
void vui_window_set_background_color( vui_handle H, uint32_t color );
void vui_window_set_title( vui_handle H, char *title );

#ifdef __cplusplus
}
#endif
#endif