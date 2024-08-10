#ifndef VIOS_VUI_EVENT_INCLUDED
#define VIOS_VUI_EVENT_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include "vui/vui.h"

#define VUI_EVENT_MOUSE_DOWN 1
#define VUI_EVENT_MOUSE_UP 2
#define VUI_EVENT_KEY_DOWN 3
#define VUI_EVENT_KEY_UP 4
#define VUI_EVENT_MOUSE_MOVE 5
#define VUI_EVENT_MOUSE_ENTER 6
#define VUI_EVENT_MOUSE_EXIT 7
#define VUI_EVENT_ALL 255

#define VUI_EVENT_FLAG_LMB		(1 << 0)
#define VUI_EVENT_FLAG_RMB		(1 << 1)
#define VUI_EVENT_FLAG_SHIFT	(1 << 2)
#define VUI_EVENT_FLAG_CTRL		(1 << 3)
#define VUI_EVENT_FLAG_ALT		(1 << 4)

void vui_external_event_handler( uint8_t event_type, uint16_t x, uint16_t y, bool lmb, bool rmb );
vui_handle vui_find_handler_for_event( vui_handle_list *list, vui_event *e );
void vui_send_event( vui_handle H, vui_event *e );
void vui_set_event_handler( vui_handle H, uint8_t event_type, void (*handler)(vui_event *) );
bool vui_is_dispatcher( uint16_t type );
char *vui_event_type_to_string( uint16_t type );


#ifdef __cplusplus
}
#endif
#endif