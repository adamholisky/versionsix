#include <kernel_common.h>
#include <device.h>
#include <vfs.h>
#include <rfs.h>
#include <ksymbols.h>

char device_registerfunc_ident[] = "device_register_";

bool devices_setup_status = false;
device_list device_head;

void device_initalize( void ) {
	device_head.dev = NULL;
	device_head.next = NULL;

	// Find all symbols that start wtih "kshell_app_add_command" and call them each
	symbol_collection *ksym = get_ksyms_object();
	symbol *symbol_array = symbols_get_symbol_array( ksym );
	uint64_t max_symbols = symbols_get_total_symbols( ksym );

	for( int i = 0; i < max_symbols; i++ ) {
		if( strncmp( symbol_array[i].name, device_registerfunc_ident, sizeof(device_registerfunc_ident) - 1 ) == 0 ) {
			debugf( "found: %s\n", symbol_array[i].name );
			void (*func)(void) = (void(*)(void))symbol_array[i].addr;
			func();
		}
	}

	devices_setup_status = true;
}

/**
 * @brief Returns if devices are ready to use or not
 * 
 * @return true 
 * @return false 
 */
bool devices_setup( void ) {
	return devices_setup_status;
}

void device_register( device *d ) {
	device_list *head = &device_head;
	device_list *tail = NULL;

	do {
		tail = head;
		head = head->next;
	} while( head != NULL );

	tail->next = kmalloc( sizeof(device_list) );
	tail = tail->next;

	tail->dev = d;
	tail->next = NULL;
}

device *device_get_major_minor_device( char *major, char *minor ) {
	device_list *head = &device_head;
	bool found = false;

	do {
		if( strcmp(head->dev->major_id, major) == 0 ) {
			if( strcmp(head->dev->minor_id, minor) == 0 ) {
				found = true;
			}
		}

		head = head->next;
	} while( head != NULL && !found );

	if( found == false ) {
		debugf( "Cannot find device %s,%s\n", major, minor );
		return NULL;
	}

	return head->dev;
}