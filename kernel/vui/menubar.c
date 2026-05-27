#include <vui/vui.h>
#include <vui/menubar.h>
#include <rtc.h>

extern vui_core vui;

vui_menubar main_menubar;
vui_handle main_menubar_handle;
char clock_display_text[25];
bool menubar_is_here;

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

	memset( clock_display_text, 25, 0 );

	vui_create_cleanup( main_menubar_handle );
	return main_menubar_handle;
}

void vui_menubar_draw( vui_handle H ) {
	vui_theme *theme = vui_get_active_theme();

	vui_draw_rect( 0, 0, vui.width, 25, theme->menubar_background );
	
	char *str = kmalloc(255);

	sprintf( str, " File     Edit     Tests     Debug    Windows"	 );
	vui_draw_string_ttf( str, 5, 5, theme->menubar_foreground, theme->menubar_background, vui_font_get_font("noto-sans-bold"), 13, VUI_DRAW_FLAGS_NONE );

	menubar_is_here = true;

	vui_menubar_update_clock();

	kfree(str);
}

void vui_menubar_update_clock( void ) {
	if( menubar_is_here ) {
		vui_theme *theme = vui_get_active_theme();

		memset( clock_display_text, 0, 25 );
		sprintf( clock_display_text, "%s", rtc_get_datetime_string() );

		//debugf( "update: %s\n", clock_display_text );

		vui_draw_rect( 1024-125, 0, 125, 25, theme->menubar_background );
		vui_draw_string_ttf( clock_display_text, 1024-125, 5, theme->menubar_foreground, theme->menubar_background, vui_font_get_font("noto-sans-bold"), 13, VUI_DRAW_FLAGS_NONE );

		vui_refresh_rect( 1024-125, 0, 125, 25 );
	}
}

void vui_menubar_draw_from_struct( vui_menubar *menubar ) {
	vui_menubar_draw(0);
}

void vui_menubar_add( vui_menubar *menubar, vui_menu *menu ) {
	
}