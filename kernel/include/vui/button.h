#ifndef VIOS_VUI_BUTTON_INCLUDED
#define VIOS_VUI_BUTTON_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include "vui/vui.h"

#define VUI_BUTTON_FLAG_NONE				0

#define VUI_BUTTON_BORDER_SIZE 1
#define VUI_BUTTON_TEXT_HEIGHT 20

#define VUI_BUTTON_TEXT_MAX 25

typedef struct {
	INSERT_VUI_COMMON

	uint32_t flags;

	uint32_t color_foreground;
	uint32_t color_background;

	
	bool is_active;

	char text[VUI_BUTTON_TEXT_MAX];


} vui_button;

vui_handle vui_button_create( char *text, uint16_t x, uint16_t y, uint16_t width, uint16_t height, vui_handle parent );
void vui_button_draw( vui_handle H );
void vui_button_draw_from_struct( vui_button *window );
void vui_window_set_color( vui_handle H, uint32_t color_foreground, uint32_t color_background );
void vui_button_set_text( vui_handle H, char *title );

void vui_button_on_mouse_move( vui_event *e );
void vui_button_on_mouse_enter( vui_event *e );
void vui_button_on_mouse_exit( vui_event *e );
void vui_button_on_mouse_up( vui_event *e );
void vui_button_on_mouse_down( vui_event *e );

#ifdef __cplusplus
}
#endif
#endif