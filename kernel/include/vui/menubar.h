#ifndef VIOS_VUI_MENUBAR_INCLUDED
#define VIOS_VUI_MENUBAR_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include "vui/vui.h"
#include "vui/menu.h"

typedef struct {
	INSERT_VUI_COMMON

	vui_menu *menus;
} vui_menubar;

vui_handle vui_menubar_create( void );
void vui_menubar_draw( vui_handle H );
void vui_menubar_draw_from_struct( vui_menubar *menubar );
void vui_menubar_add( vui_menubar *menubar, vui_menu *menu );

#ifdef __cplusplus
}
#endif
#endif