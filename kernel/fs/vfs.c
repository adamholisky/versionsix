#include <kernel_common.h>
#include <kmemory.h>
#include <ahci.h>
#include <fs.h>
#include <vfs.h>
#include <lib/list.h>

// TODO: Remove this, testing only
#include <asvfs.h>
#include <asvfs_vios.h>

avs_list* file_systems;
avs_list* mount_points;
avs_list* vfs_inodes;

int fs_id_next = 1;

vfs_inode root_inode;
vfs_mount_point root_mount_point;

/**
 * @brief Initalizes the VFS
 *
 * @return int VFS_ERROR_NONE on success, otherwise VFS_ERROR_ on failure
 */
int vfs_initalize( void ) {
	file_systems = avs_list_init();
	mount_points = avs_list_init();
	vfs_inodes = avs_list_init();

	root_inode.fs_type = 0;
	root_inode.type = VFS_INODE_TYPE_DIR;
	root_inode.id = 1;
	root_inode.is_mount_point = false;

	memset( &root_mount_point, 0, sizeof(vfs_mount_point) );
	root_mount_point.drive_id = 0;
	strcpy( root_mount_point.fs_type, "UNSET" );
	root_mount_point.root[0] = '/';

	// vfs_cache_initalize();

	return VFS_ERROR_NONE;
}

vfs_filesystem* vfs_register_fs( char* fs_name, vfs_operations* fs_ops ) {
	vfs_filesystem* fs = kmalloc( sizeof( vfs_filesystem ) );

	strcpy( fs->name, fs_name );

	fs->op.create = fs_ops->create;
	fs->op.read = fs_ops->read;
	fs->op.write = fs_ops->write;
	fs->op.getattr = fs_ops->getattr;
	fs->op.get_dir_list = fs_ops->get_dir_list;
	fs->op.open = fs_ops->open;

	avs_list_append( file_systems, fs );

	return fs;
}

// TODO: Replace drive id with device id
int vfs_mount( char* fs_type, char* mount_path, int drive_id ) {
	vfs_mount_point* mp = kmalloc( sizeof( vfs_mount_point ) );

	strcpy( mp->fs_type, fs_type );
	strcpy( mp->root, mount_path );
	mp->drive_id = drive_id;

	avs_node *n = avs_list_find_data( file_systems, fs_type, avs_list_compare_fs_type );

	if( n != NULL ) {
		mp->fs = (vfs_filesystem *)n->data;
	} else {
		klog( LOG_ERROR, "couldn't find fs data for fs_type \"%s\"", fs_type );
	}

	avs_list_append( mount_points, mp );
}

/**
 * @brief Finds the FS responsible for the given pathname
 *
 * @param pathname
 * @return vfs_mount_point* pointer to the FS, NULL on any error
 */
vfs_mount_point* vfs_get_mount_point_from_path( char* pathname ) {
	char path_elements[25][256];
	int current_path_ele_index = 0;

	int name_index = 0;
	int path_index = 0;

	bool keep_going_main = true;
	bool keep_going_secondary = true;
	char* c = pathname;
	int path_length = strlen( c );

	memset( path_elements, 0, 25 * 256 );

	do {
		do {
			if ( *c == '/' || *c == 0 || path_index == path_length ) {
				if ( path_index == 0 ) {
					path_elements[0][0] = '/';
				}

				if ( path_index == path_length ) {
					keep_going_main = false;
				}

				keep_going_secondary = false;
				current_path_ele_index++;
				name_index = 0;
			}
			else {
				path_elements[current_path_ele_index][name_index++] = *c++;
				path_index++;
			}
		} while ( keep_going_secondary );

		c++;
		path_index++;
		//printf( "path_elements[%d] = %s\n", current_path_ele_index - 1, path_elements[current_path_ele_index - 1] );

		if ( path_index >= path_length ) {
			keep_going_main = false;
		}
		else {
			keep_going_secondary = true;
		}
	} while ( keep_going_main );

	char path_to_test[255];
	char farthest_mount_point_path[255];
	vfs_mount_point *farthest_mount_point = NULL;
	
	memset( path_to_test, 0, 255 );
	strcpy( path_to_test, path_elements[0] );

	for( int n = 0; n < current_path_ele_index; n++ ) {
		if( n == 0 && path_to_test[0] == '/' ) {
			farthest_mount_point_path[0] = '/';

			avs_node *n_mp = avs_list_find_data( mount_points, "/", avs_list_compare_mount_point_roots );

			farthest_mount_point == n_mp->data;
		} else {
			avs_node *n_mp = avs_list_find_data( mount_points, path_to_test, avs_list_compare_mount_point_roots );

			if( n_mp != NULL ) {
				farthest_mount_point = n_mp->data;
				strcpy( farthest_mount_point_path, path_to_test );
			}
		}

		//debugf( "n: %d  ptt: \"%s\"  fmpp: \"%s\"  fmp: %X\n", n, path_to_test, farthest_mount_point_path, farthest_mount_point );

		if( n + 1 < current_path_ele_index ) {
			if( n > 0 ) {
				strcat( path_to_test, "/" );
			}

			strcat( path_to_test, path_elements[n + 1] );
		}
	}

	return farthest_mount_point;
}

int avs_list_compare_fs_type( void *a, void *b ) {
	return strcmp( (char *)a, ((vfs_filesystem *)b)->name );
}

int avs_list_compare_mount_point_roots( void *a, void *b ) {
	//klog( LOG_INFO, "comparing a=\"%s\"  b=\"%s\"", (char *)a, ((vfs_mount_point *)b)->root );

	return strcmp( (char *)a, ((vfs_mount_point *)b)->root );
}

int vfs_create( const char *pathname, mode_t mode ) {
	vfs_mount_point *mp = vfs_get_mount_point_from_path( pathname );

	if( mp == NULL ) {
		klog( LOG_ERROR, "MP is null. pathname=%s mode=%d", pathname, mode );
		return -1;
	}

	return mp->fs->op.create( pathname, mode );
}

int vfs_write( const char *pathname, char *buff, off_t offset, size_t length ) {
	return asvfs_write( pathname, buff, offset, length );
}

int vfs_read( const char *pathname, char *buff, off_t offset, size_t length ) {
	return asvfs_read( pathname, buff, offset, length );
}

int vfs_getattr( const char *pathname, file_stats *stbuff ) {
	return asvfs_getattr( pathname, stbuff );
}


/** VFS->HW Interfaces */

int vfs_disk_read( uint64_t drive, uint64_t offset, uint64_t length, uint8_t *data ) {
	klog( LOG_DEBUG, "vfs_read_from_disk: drive=%d offset=%X length=%X data=%X", drive, offset, length, data );
	return vfs_disk_read_no_cache( drive, offset, length, data );
}

int vfs_disk_read_no_cache( uint64_t drive, uint64_t offset, uint64_t length, uint8_t *data ) {
	if( !ahci_read_at_byte_offset_512_chunks( offset, length, data ) ) {
		klog( LOG_ERROR, "Could not read from ahci drive.\n" );
		return VFS_ERROR_UNKNOWN;
	}

	return VFS_ERROR_NONE;
}

uint8_t *vfs_disk_write( uint64_t drive, uint64_t offset, uint64_t length, uint8_t *data ) {
	return vfs_disk_write_no_cache( drive, offset, length, data );
}

uint8_t *vfs_disk_write_no_cache( uint64_t drive, uint64_t offset, uint64_t length, uint8_t *data ) {
	return 0;
}