#include <vui/vui.h>
#include <vui/event.h>

extern vui_core vui;

/**
 * @brief 
 * 
 * @param x 
 * @param y 
 * @param lmb 
 * @param rmb 
 */
void vui_external_event_handler( uint8_t event_type, uint16_t x, uint16_t y, bool lmb, bool rmb ) {
	vui_event event;
	memset( &event, 0, sizeof(event) );

	event.x = x;
	event.y = y;
	event.type = event_type;
	event.flags = (lmb == true ? event.flags | VUI_EVENT_FLAG_LMB : event.flags);
	event.flags = (rmb == true ? event.flags | VUI_EVENT_FLAG_RMB : event.flags);

	// Step 1: check if it's in the active dispatcher (window, alert, etc...)

	// Step 2: If not, search all dispatchers
	vui_handle_list *top = &vui.dispatchers;
	vui_handle dispatcher = vui_find_handler_for_event( &vui.dispatchers, &event );

	if( dispatcher != 0 ) {
		vui_common *vc_dispatcher = vui_get_handle_data(dispatcher);
		vui_handle handler = vui_find_handler_for_event( &vc_dispatcher->children, &event );

		// If there's a child to send the event to, do it
		// Otherwise send to the dispatcher
		if( handler != 0 ) {
			event.H = handler;
			strcpy( event.name, vui_handle_get_name(event.H) );
			vui_send_event( handler, &event );
		} else {
			event.H = dispatcher;
			strcpy( event.name, vui_handle_get_name(event.H) );
			vui_send_event( dispatcher, &event );
		}

		// If there's an event handler present on the dispatcher, call that
		if( vc_dispatcher->ops.event_handler != NULL ) {
			vc_dispatcher->ops.event_handler( &event );
		}
	}
}

/**
 * @brief 
 * 
 * @param list 
 * @param e 
 * @return vui_handle 
 */
vui_handle vui_find_handler_for_event( vui_handle_list *list, vui_event *e ) {
	vui_handle_list *top = list;
	bool found_dispatcher = false;
	vui_common *element;

	do {
		if( top->H == 0 ) {
			// End of list reached, just return, we're not finding anything to send to
			return 0;
		}

		element = vui_get_handle_data(top->H);

		if( (e->x >= element->absolute_x) && (e->x <= element->absolute_x + element->width) ) {
			if( (e->y >= element->absolute_y) && (e->y <= element->absolute_y + element->height) ) {
				found_dispatcher = true;
			}
		}

		top = top->next;
	} while( top != NULL && !found_dispatcher );

	if( found_dispatcher ) {
		//vdf( "Found handler:  H = %d    Type: %d\n", element->handle, element->type );
		return element->handle;
	}

	return 0;
}

/**
 * @brief 
 * 
 * @param H 
 * @param e 
 */
void vui_send_event( vui_handle H, vui_event *e ) {
	//vdf( "Handler got event. H = %d    e.flags = %d    e.type = %d\n", H, e->flags, e->type );

	vui_common *element = vui_get_handle_data(H);

	void (*handler_to_call)(vui_event *) = NULL;

	// Handle mouse exiting
	if( e->type == VUI_EVENT_MOUSE_MOVE ) {
		if( vui.last_hover != 0) {
			if( vui.last_hover != e->H ) {
				vui_common *hov_elem = vui_get_handle_data(vui.last_hover);

				e->H = vui.last_hover;
				e->type = VUI_EVENT_MOUSE_EXIT;
				element = vui_get_handle_data(e->H);
				strcpy( e->name, element->name );	

				vui.last_hover = 0;
			} else {
				// This is a mouse move event, inside the control
				// Do nothing
			}
		} else {
			// This is a mouse enter event
			e->type = VUI_EVENT_MOUSE_ENTER;
			
			vui.last_hover = e->H;
		}

	}
	
	switch( e->type ) {
		case VUI_EVENT_MOUSE_MOVE:
			handler_to_call = element->ops.default_on_mouse_move;
			break;
		case VUI_EVENT_MOUSE_EXIT:
			handler_to_call = element->ops.default_on_mouse_exit;
			break;
		case VUI_EVENT_MOUSE_ENTER:
			handler_to_call = element->ops.default_on_mouse_enter;
			break;
		case VUI_EVENT_MOUSE_DOWN:
			handler_to_call = element->ops.default_on_mouse_down;
			break;
		case VUI_EVENT_MOUSE_UP:
			handler_to_call = element->ops.default_on_mouse_up;
			break;
	}

	if( handler_to_call != NULL ) {
		handler_to_call(e);
	} else {
		// set hover if current thing can't set it?
		/* if( e->type == VUI_EVENT_MOUSE_MOVE ) {
			vui.last_hover = e->H;
		} */
	}
}

/**
 * @brief 
 * 
 * @param H 
 * @param event_type 
 * @param handler 
 */
void vui_set_event_handler( vui_handle H, uint8_t event_type, void (*handler)(vui_event *) ) {
	vui_common *element = vui_get_handle_data(H);

	if( element == NULL ) {
		vdf( "Element was NULL in set_event_handler. Aborting.\n" );
		return;
	}

	switch( event_type ) {
		case VUI_EVENT_ALL:
			element->ops.event_handler = handler;
			break;
		case VUI_EVENT_MOUSE_DOWN:
			element->ops.on_mouse_down = handler;
			break;
		case VUI_EVENT_MOUSE_UP:
			element->ops.on_mouse_up = handler;
			vdf( "set on mouse up\n" );
			break;
	}
}

/**
 * @brief 
 * 
 * @param type 
 * @return true 
 * @return false 
 */
bool vui_is_dispatcher( uint16_t type ) {
	switch( type ) {
		case VUI_HANDLE_TYPE_DESKTOP:
		case VUI_HANDLE_TYPE_WINDOW:
		case VUI_HANDLE_TYPE_MENUBAR:
			return true;
			break;
	}

	return false;
}

char *vui_event_type_to_string( uint16_t type ) {
	switch( type ) {
		case VUI_EVENT_MOUSE_UP:
			return "Mouse Up";
			break;
		case VUI_EVENT_MOUSE_DOWN:
			return "Mouse Down";
			break;
		case VUI_EVENT_KEY_UP:
			return "Key Up";
			break;
		case VUI_EVENT_KEY_DOWN:
			return "Key Down";
			break;
		case VUI_EVENT_MOUSE_ENTER:
			return "Mouse Enter";
			break;
		case VUI_EVENT_MOUSE_EXIT:
			return "Mouse Exit";
			break;
		default:
			char str[50];
			sprintf( str, "Unknown Event Type %d", type );
			return str;
	}
}