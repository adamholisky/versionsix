#include <vui/vui.h>
#include <vui/menubar.h>

extern vui_core vui;

vui_menubar main_menubar;
vui_handle main_menubar_handle;

vui_handle vui_menubar_create( void ) {
	main_menubar_handle = vui_allocate_handle( VUI_HANDLE_TYPE_MENUBAR );

	memset( &main_menubar, 0, sizeof(vui_menubar) );
	main_menubar.type = VUI_HANDLE_TYPE_MENUBAR;
	main_menubar.handle = main_menubar_handle;
	main_menubar.priority = 0xFFFFFFFE;

	main_menubar.x = 0;
	main_menubar.y = 0;
	main_menubar.width = vui.width;
	main_menubar.height = 25;

	vui_set_handle_data( main_menubar_handle, &main_menubar );

	vui_create_cleanup( main_menubar_handle );
	return main_menubar_handle;
}

void vui_menubar_draw( vui_handle H ) {
	vui_theme *theme = vui_get_active_theme();

	vui_draw_rect( 0, 0, vui.width, 25, theme->menubar_background );
	
	char str[255];

	sprintf( str, " Vi  |  File     Edit     Tests     Debug"	 );
	vui_draw_string( str, 5, 5, theme->menubar_foreground, theme->menubar_background, vui_font_get_font("Zap VGA"), VUI_DRAW_FLAGS_NONE );

}

void vui_menubar_draw_from_struct( vui_menubar *menubar ) {
	vui_menubar_draw(0);
}

void vui_menubar_add( vui_menubar *menubar, vui_menu *menu ) {
	
}