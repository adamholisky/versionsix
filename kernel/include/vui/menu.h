#ifndef VIOS_VUI_MENU_INCLUDED
#define VIOS_VUI_MENU_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include "vui/vui.h"
#include "vui/menubar.h"

typedef struct {
	INSERT_VUI_COMMON

	char text[50];

	void *next;
} vui_menu_item;

typedef struct {
	INSERT_VUI_COMMON

	char text[50];

	vui_menu_item *items;
	uint8_t num_items;

	void *next;
} vui_menu;

vui_handle vui_menu_create( char *name, char *text );
void vui_menu_draw( vui_handle H );
void vui_menu_draw_from_struct( vui_menu *menu );
void vui_menu_add_item( vui_menu *menu, char *name, char *text );
void vui_menu_calculate_dimensions( vui_menu *menu );

#ifdef __cplusplus
}
#endif
#endif