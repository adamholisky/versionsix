#include <kernel_common.h>
#include <device.h>
#include <vfs.h>
#include <rfs.h>
#include <ksymbols.h>
#include <kmemory.h>
#include <lib/list.h>

char device_registerfunc_ident[] = "device_register_";

bool devices_setup_status = false;
avs_list *device_list;

void device_initalize( void ) {
	device_list = avs_list_init();

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

void devices_populate_fs( void ) {
	avs_list_for_each( device_list, device_list_populate_for_each_callback );
}

void device_list_populate_for_each_callback( avs_node *n ) {
	device *dev = n->data;

	if( dev == NULL ) {
		return;
	}

	char name[50];

	memset( &name, 0, 50 );
	strcpy( name, dev->major_id );
	
	if( dev->minor_id[0] != '0' ) {
		strcat( name, dev->minor_id );
	}

	char pathname[255];
	memset( pathname, 0, 255 );
	strcpy( pathname, "/dev/" );
	strcat( pathname, name );
	int cr_err = vfs_create( pathname, 0 );

	if( cr_err != VFS_ERROR_NONE ) {
		klog( LOG_ERROR, "vfs_create error on \"%s\": %d", pathname, cr_err );
	} else {
		klog( LOG_INFO, "created device: \"%s\"", pathname );
	}
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
	avs_list_append( device_list, d );
}

device *device_get_major_minor_device( char *major, char *minor ) {
	avs_node *h = device_list->head;
	bool found = false;
	device *d = NULL;

	do {
		device *d_temp = (device *)h->data;
		if( strcmp(d_temp->major_id, major) == 0 ) {
			if( strcmp(d_temp->minor_id, minor) == 0 ) {
				found = true;
				d = d_temp;
			}
		}

		h = h->next;
	} while( h != NULL && !found );
	
	if( found == false ) {
		debugf( "Cannot find device %s,%s\n", major, minor );
		return NULL;
	}
	
	return d;
}

