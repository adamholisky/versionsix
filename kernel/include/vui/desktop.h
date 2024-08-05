#ifndef VIOS_VUI_DESKTOP_INCLUDED
#define VIOS_VUI_DESKTOP_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include "vui/vui.h"

#define VUI_DESKTOP_FLAG_NONE				0

typedef struct {
	uint16_t handle;
	uint32_t flags;
	
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;

	uint32_t color_background;
} vui_desktop;

vui_handle vui_desktop_create( uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t flags );
void vui_desktop_draw( vui_handle H );
void vui_desktop_draw_from_struct( vui_desktop *desktop );
void vui_desktop_set_background_color( vui_handle H, uint32_t color );

#ifdef __cplusplus
}
#endif
#endif