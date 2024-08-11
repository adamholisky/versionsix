#ifndef VIOS_VUI_LABEL_INCLUDED
#define VIOS_VUI_LABEL_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include "vui/vui.h"

#define VUI_LABEL_FLAG_NONE				0
#define VUI_LABEL_FLAG_NO_SMOOTHING		(1 << 0)
#define VUI_LABEL_FLAG_TRANSPARENT		(1 << 1)

typedef struct {
	INSERT_VUI_COMMON

	uint32_t flags;

	uint32_t color_foreground;
	uint32_t color_background;

	vui_font *font;

	char *text;
} vui_label;

vui_handle vui_label_create( uint16_t x, uint16_t y, char *text, uint32_t flags, vui_handle parent );
void vui_label_draw( vui_handle H );
void vui_label_draw_from_struct( vui_label *lable );
void vui_label_set_color( vui_handle H, uint32_t foreground, uint32_t background );
void vui_label_set_font( vui_handle H, vui_font *font );

#ifdef __cplusplus
}
#endif
#endif