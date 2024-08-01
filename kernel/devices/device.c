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

void devices_populate_fs( void ) {
	device_list *head = &device_head;

	do {
		char name[50];

		memset( &name, 0, 50 );
		strcpy( name, head->dev->major_id );
		strcat( name, head->dev->minor_id );
		inode_id id = vfs_create( VFS_INODE_TYPE_FILE, "/dev", name );
		
		vfs_inode *ino = vfs_lookup_inode_ptr_by_id(id);
		if( ino == NULL ) {
			debugf( "ino returned null!\n" );
			return;
		}

		// TODO: This should live in vfs_create code
		ino->dev = kmalloc( sizeof(vfs_device) );
		
		ino->dev->data = head->dev;

		head = head->next;
	} while( head != NULL );
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
	device_list *dl = NULL;
	bool found = false;

	do {
		if( head->dev == NULL ) {
			dl = head;
			found = true;
		} else {
			if( head->next != NULL ) {
				head = head->next;
			} else {
				head->next = kmalloc( sizeof(device_list) );
				dl = head->next;
				found = true;
			}
		}
		tail = head;
		head = head->next;
	} while( head != NULL && !found );

	if( dl == NULL ) {
		debugf( "Cannot find free device.\n" );
		return;
	}

	dl->dev = d;
	dl->next = NULL;
}

device *device_get_major_minor_device( char *major, char *minor ) {
	device_list *head = &device_head;
	device_list *dl = NULL;
	bool found = false;

	do {
		if( strcmp(head->dev->major_id, major) == 0 ) {
			if( strcmp(head->dev->minor_id, minor) == 0 ) {
				found = true;
				dl = head;
			}
		}

		head = head->next;
	} while( head != NULL && !found );

	if( found == false ) {
		debugf( "Cannot find device %s,%s\n", major, minor );
		return NULL;
	}

	return dl->dev;
}