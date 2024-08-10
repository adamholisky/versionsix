#include <vui/vui.h>
#include <vui/menubar.h>
#include <vui/menu.h>

vui_handle vui_menu_create( char *name, char *text ) {
	vui_handle H = vui_allocate_handle( VUI_HANDLE_TYPE_MENU );
	vui_menu *menu = vmalloc( sizeof(vui_menu) );
	memset( menu, 0, sizeof(vui_menu) );

	menu->type = VUI_HANDLE_TYPE_MENU;
	menu->handle = H;
	menu->priority = 0x1;
	menu->next = NULL;
	strcpy( menu->text, text );
	strcpy( menu->name, name );

	vui_set_handle_data( H, menu );
	vui_create_cleanup( H );
	return H;
}

void vui_menu_draw( vui_handle H ) {
	vui_menu_draw_from_struct( vui_get_handle_data(H) );
}

void vui_menu_draw_from_struct( vui_menu *menu ) {
	vui_theme *theme = vui_get_active_theme();
	
	vui_menu_calculate_dimensions( menu );

	vui_draw_rect( menu->x, menu->y, menu->width, menu->height, theme->border );
	vui_draw_rect( menu->inner_x, menu->inner_y, menu->inner_width, menu->inner_height, theme->menubar_background ); 

	for( int i = 0; i < menu->num_items; i++ ) {
		vui_draw_string( menu->items[i].text, menu->inner_x + 5, menu->inner_y + (i * 25) + 5, theme->menubar_foreground, theme->menubar_background, vui_font_get_font("Zap VGA"), true );
	}
}

void vui_menu_set_display_location( vui_menu *menu, uint16_t x, uint16_t y ) {
	menu->absolute_x = x;
	menu->absolute_y = y;
	menu->x = x;
	menu->y = y;
}

void vui_menu_calculate_dimensions( vui_menu *menu ) {
	menu->width = 50 + 10 + 2; // 50 content, 10 = 5 padding on right left, 2 = border
	menu->height = (menu->num_items * 25) + 10 + 2; // 25 per item, 5 padding on top bottom, 2 = border

	menu->inner_x = menu->x - 1;
	menu->inner_y = menu->y - 1;
	menu->inner_width = menu->width - 2;
	menu->inner_height = menu->height - 2;
}

void vui_menu_add_item( vui_menu *menu, char *name, char *text ) {
	vui_menu_item *head = menu->items;
	vui_menu_item *item = NULL;

	if( head == NULL ) {
		head = vmalloc(sizeof(vui_menu_item));
		item = head;
	} else {
		do {
			if( head->next == NULL ) {
				head->next = vmalloc(sizeof(vui_menu_item));
				item = head->next;
			}
			head = head->next;
		} while( item != NULL );
	}
	
	strcpy( head->text, text );
	strcpy( head->name, name );
	head->next = NULL;
}
