#include "vit.h"
#include "vui/vui.h"
#include "vui/event.h"
#include "vui/font.h"
#include "vui/desktop.h"
#include "vui/window.h"
#include "vui/label.h"
#include "vui/console.h"
#include "vui/button.h"
#include "vui/menubar.h"

vui_core vui;

/**
 * @brief Initalize the VUI GUI system
 * 
 * @param fb_addr Address of the FB that outputs to the display
 * @param width Width of display 
 * @param height Height of display
 */
void vui_init( uint32_t *fb_addr, uint16_t width, uint16_t height ) {
	vui.fb = fb_addr;
	vui.width = width;
	vui.height = height;
	vui.pitch = width * 4;
	vui.buffer = vmalloc( 4 * width * height );
	vui.handle_next = 1;

	vui.immediate_output = false;
	vui.disable_font_smoothing = false;

	memset( vui.buffer, 0, 4 * width * height );

	vui.active_theme.desktop = 0x00374760;
	vui.active_theme.border = 0x747474;
	vui.active_theme.window_background = 0x00D6D6D6;
	vui.active_theme.window_title_bar_background = 0x00363636;
	vui.active_theme.window_title_bar_foreground = 0x00EAEAEA;
	vui.active_theme.button_foreground = 0xD6D6D6;
	vui.active_theme.button_background = 0x0056C1;	
	vui.active_theme.button_hover = 0x0064E1;
	vui.active_theme.button_active = 0x003476;
	vui.active_theme.menubar_background = 0xD9D9D9;
	vui.active_theme.menubar_foreground = 0x363636;
}

/**
 * @brief 
 * 
 * @param type 
 * @return vui_handle 
 */
vui_handle vui_allocate_handle( uint16_t type ) {
	if( vui.handle_next >= VUI_HANDLES_MAX ) {
		vdf( "VUI: Reached max handles!\n" );
		return 0;
	}

	vui.handles[vui.handle_next].data = NULL;
	vui.handles[vui.handle_next].type = type;
	vui.handles[vui.handle_next].H = vui.handle_next;

	// If the handle that's being added is a dispatcher, then add it to the list
	if( vui_is_dispatcher(type) ) {	
		vui_handle_list_add( &vui.dispatchers, vui.handle_next );
	}

	vui.handle_next++;

	return vui.handle_next - 1;
}



/**
 * @brief 
 * 
 * @param H 
 */
void vui_create_cleanup( vui_handle H ) {
	vui_common *vc = vui_get_handle_data(H);

	if( vc == NULL ) {
		vdf( "vui_common is NULL. Big problem. Aborting.\n" );
		return;
	}

	if( vui_is_dispatcher(vc->type) ) {
		vui_sort_list_by_priority( &vui.dispatchers );
	}
}

/**
 * @brief 
 * 
 * @param H 
 * @return uint16_t 
 */
uint16_t vui_get_type_from_master_list( vui_handle H ) {
	return vui.handles[H].type;
}

/**
 * @brief Sets the internal name of the element
 * 
 * @param H 
 * @param name 
 */
void vui_handle_set_name( vui_handle H, char *name ) {
	vui_common *vc = vui_get_handle_data(H);

	strcpy(vc->name, name);
}

/**
 * @brief Returns the name of the handle
 * 
 * @param H 
 * @return char* 
 */
char *vui_handle_get_name( vui_handle H ) {
	vui_common *vc = vui_get_handle_data(H);

	return vc->name;
}

/**
 * @brief Sets the handle's data
 * 
 * @param H Assigned handle
 * @param data Pointer to the data
 */
void vui_set_handle_data( vui_handle H, void *data ) {
	vui.handles[H].data = data;
}

/**
 * @brief Draws the given handle and all its children
 * 
 * @param H vui_handle of the element to draw
 */
void vui_draw( vui_handle H ) {
	// Step 1: Draw the container
	vui_draw_handle(H);

	// Step 2: Iterate through children, draw each
	vui_common *parent_st = vui_get_handle_data(H);

	if( parent_st == NULL ) {
		vdf( "Parent struct is null. Aborting.\n" );
		return;
	}

	vui_handle_list *top = &parent_st->children;

	do {
		if( top->H != 0 ) {
			vui_draw_handle( top->H );
		}
		
		top = top->next;
	} while( top != NULL );
}

/**
 * @brief Adds the given handle to the provided handle list
 * 
 * @param list Pointer to the list to add to
 * @param handle_to_add Handle to add 
 */
bool vui_handle_list_add( vui_handle_list *list, vui_handle handle_to_add ) {
	vui_handle_list *top = list;
	vui_handle_list *node = NULL;
	bool free_node_found = false;

	if( top == NULL ) {
		vdf( "Top is NULL. Aborting.\n" );
		return false;
	}

	do {
		if( top->next == NULL ) {
			if( top->H == 0 ) {
				node = top;
			} else {
				top->next = vmalloc( sizeof(vui_handle_list) );
				node = top->next;
			}

			free_node_found = true;
		} else {
			top = top->next;
		}
	} while( top != NULL && !free_node_found );

	if( node == NULL ) {
		vdf( "Free node returned as null. Aborting.\n" );
		return false;
	}

	node->H = handle_to_add;
	node->next = NULL;

	return true;
}

/**
 * @brief Adds the child handle to the parent
 * 
 * @param parent 
 * @param child 
 */
void vui_add_to_parent( vui_handle parent, vui_handle child ) {
	// Step 1: Get the data structs as vui_common
	vui_common *parent_st = vui_get_handle_data(parent);
	vui_common *child_st = vui_get_handle_data(child);

	if( parent_st == NULL ) {
		vdf( "Parent_st is NULL. Aborting.\n" );
		return;
	}

	if( child_st == NULL ) {
		vdf( "Child_st is NULL. Aborting.\n" );
		return;
	}

	// Step 2: Find the end of the list, add to it
	vui_handle_list *top = &parent_st->children;
	vui_handle_list_add( &parent_st->children, child_st->handle );
	child_st->parent = parent_st->handle;

	// Step 4: Sort parent's child handles by priority order
	// TODO ^^
}

/**
 * @brief Sorts the provided list by priority
 * 
 * @param list 
 */
void vui_sort_list_by_priority( vui_handle_list *list ) {
	bool keep_going = true;
	bool did_sort = false;

	//vdf( "Priority List Pre Sort:\n" );
	for( vui_handle_list *hl = list; hl != NULL; hl = hl->next ) {
		vui_common *vc = vui_get_handle_data(hl->H);
		//vdf( "   %d -> %08X\n", hl->H, vc->priority );
	}

	if( list->H == 0 ) {
		return;
	}

	if( list->next == NULL ) {
		return;
	}

	do {
		vui_handle_list *top = list;
		vui_handle_list *current = list->next;
		keep_going = true;
		did_sort = false;

		while( keep_going ) {
			vui_common *top_st = vui_get_handle_data(top->H);
			vui_common *current_st = vui_get_handle_data(current->H);
			vui_handle temp = 0;

			if( current_st->priority < top_st->priority ) {
				temp = top->H;
				top->H = current->H;
				current->H = temp;

				did_sort = true;
			}

			if( current->next == NULL ) {
				keep_going = false;
			} else {
				top = current;
				current = top->next;
			}
		}
	} while( did_sort );

	//vdf( "Priority List Post Sort:\n" );
	for( vui_handle_list *hl = list; hl != NULL; hl = hl->next ) {
		vui_common *vc = vui_get_handle_data(hl->H);
		//vdf( "   %d -> %08X\n", hl->H, vc->priority );
	}
}
