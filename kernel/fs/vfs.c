#include <kernel_common.h>
#include <kmemory.h>
#include <ahci.h>
#include <fs.h>
#include <vfs.h>
#include <lib/list.h>
#include <device.h>

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

	vfs_fd_setup();

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

	kstrcpy( mp->fs_type, fs_type );
	kstrcpy( mp->root, mount_path );
	mp->drive_id = drive_id;

	avs_node *n = avs_list_find_data( file_systems, fs_type, avs_list_compare_fs_type );

	if( n != NULL ) {
		mp->fs = (vfs_filesystem *)n->data;
	} else {
		klog( LOG_ERROR, "couldn't find fs data for fs_type \"%s\"", fs_type );
	}

	klog( LOG_INFO, "Appended mount point. fs_type=%s  mount_path=\"%s\"  drive_id=%d", mp->fs_type, mp->root, mp->drive_id );

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
	int path_length = kstrlen( c );

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
		
		path_index++;

		if ( path_index >= path_length ) {
			keep_going_main = false;
		}
		else {
			c++;
			keep_going_secondary = true;
		}
	} while ( keep_going_main );

	char path_to_test[255];
	char farthest_mount_point_path[255];
	vfs_mount_point *farthest_mount_point = NULL;
	
	memset( path_to_test, 0, 255 );
	kstrcpy( path_to_test, path_elements[0] );

	for( int n = 0; n < current_path_ele_index; n++ ) {
		if( n == 0 && path_to_test[0] == '/' ) {
			farthest_mount_point_path[0] = '/';

			avs_node *n_mp = avs_list_find_data( mount_points, "/", avs_list_compare_mount_point_roots );

			if( n_mp == NULL ) {
				//klog( LOG_ERROR, "mount points var is null" );
				printf( "mount points var is null\n" );
				return NULL;
			}

			farthest_mount_point = (vfs_mount_point *)n_mp->data;
		} else {
			avs_node *n_mp = avs_list_find_data( mount_points, path_to_test, avs_list_compare_mount_point_roots );

			if( n_mp != NULL ) {
				farthest_mount_point = (vfs_mount_point *)n_mp->data;
				kstrcpy( farthest_mount_point_path, path_to_test );
			}
		}

		if( n + 1 < current_path_ele_index ) {
			if( n > 0 ) {
				kstrcat( path_to_test, "/" );
			}

			kstrcat( path_to_test, path_elements[n + 1] );
		}
	}

	return farthest_mount_point;
}

int avs_list_compare_fs_type( void *a, void *b ) {
	return strcmp( (char *)a, ((vfs_filesystem *)b)->name );
}

int avs_list_compare_mount_point_roots( void *a, void *b ) {
	//klog( LOG_INFO, "comparing a=\"%s\"  b=\"%s\"", (char *)a, ((vfs_mount_point *)b)->root );

	char *path_to_test = a;
	vfs_mount_point *mp_to_test_again = b;
	return kstrcmp( path_to_test, mp_to_test_again->root );
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
	vfs_mount_point *mp = vfs_get_mount_point_from_path( pathname );

	if( mp == NULL ) {
		klog( LOG_ERROR, "MP is null. pathname=%s buff=%X, offset=%d, len=%d", pathname, buff, offset, length );
		return -1;
	}

	// device hooks
	file_stats stats;
	memset( &stats, 0, sizeof(file_stats) );

	mp->fs->op.getattr( pathname, &stats );
	if( stats.type == VFS_INODE_TYPE_DEVICE ) {
		vfs_device_data device_data;

		mp->fs->op.read( pathname, &device_data, 0, sizeof(vfs_device_data) );
		device *dev = device_get_major_minor_device( device_data.major, device_data.minor );

		klog( LOG_DEBUG, "major: %s  minor: %s", device_data.major, device_data.minor );

		return dev->write( 0, buff, length, offset );
	}

	return mp->fs->op.write( pathname, buff, offset, length );
}

int vfs_write_device_meta( char *pathname, char *major_id, char *minor_id ) {
	vfs_mount_point *mp = vfs_get_mount_point_from_path( pathname );

	if( mp == NULL ) {
		klog( LOG_ERROR, "MP is null. pathname=%s major_id=%s, minor_id=%s", pathname, major_id, minor_id );
		return -1;
	}

	// device hooks
	file_stats stats;
	memset( &stats, 0, sizeof(file_stats) );

	vfs_device_data device_data;
	strcpy( device_data.major, major_id );
	strcpy( device_data.minor, minor_id );

	//klog( LOG_INFO, "Wrote device_data: major=%s  minor=%s. fs=%s", device_data.major, device_data.minor, mp->fs->name );

	return mp->fs->op.write( pathname, &device_data, 0, sizeof(vfs_device_data) );
}

int vfs_read( const char *pathname, char *buff, size_t size, off_t offset ) {
	vfs_mount_point *mp = vfs_get_mount_point_from_path( pathname );

	if( mp == NULL ) {
		klog( LOG_ERROR, "MP is null. pathname=%s buff=%X, size=%d, offset=%d", pathname, buff, size, offset );
		return -1;
	}

	return mp->fs->op.read( pathname, buff, size, offset );
}

int vfs_getattr( const char *pathname, file_stats *stbuff ) {
	return asvfs_getattr( pathname, stbuff );
}

vfs_dir *vfs_opendir( char *pathname ) {
	file_stats st;
	int getattr_err = asvfs_getattr( pathname, &st );
	if( getattr_err != VFS_ERROR_NONE ) {
		klog( LOG_ERROR, "Getattr returned an error: %d", getattr_err );
		return NULL;
	}

	vfs_dir *dirp = kmalloc( sizeof(vfs_dir) );
	dirp->count = 0;

	int get_dir_list_err = asvfs_get_dir_list_glue( pathname, dirp );
	if( get_dir_list_err != VFS_ERROR_NONE ) {
		klog( LOG_ERROR, "asvfs_get_dir_list_glue returned an error: %d", get_dir_list_err );

		kfree( dirp );
		return NULL;
	}

	dirp->next = 0;
	
	return dirp;
}

void vfs_closedir( vfs_dir *dirp ) {
	int close_dir_err = asvfs_close_dir_list_glue( dirp );
	if( close_dir_err != VFS_ERROR_NONE ) {
		klog( LOG_ERROR, "asvfs_close_dir_list_glue returned an error: %d", close_dir_err );
	}

	kfree( dirp );
}

vfs_dirent *vfs_readdir( vfs_dir *dirp ) {
	avs_node *n = avs_list_at_index_node( dirp->dir_list, dirp->next );
	if( n == NULL ) {
		return NULL;
	}

	dirp->next++;

	return (vfs_dirent *)n->data;
}


/** VFS->HW Interfaces */

int vfs_disk_read( uint64_t drive, uint8_t *buff, size_t size, off_t offset ) {
	//klog( LOG_DEBUG, "vfs_read_from_disk: drive=%d offset=%X length=%X data=%X", drive, offset, length, data );
	return vfs_disk_read_no_cache( drive, buff, size, offset );
}

int vfs_disk_read_no_cache( uint64_t drive, uint8_t *buff, size_t size, off_t offset ) {
	if( !ahci_read_at_byte_offset_512_chunks( offset, size, buff ) ) {
		klog( LOG_ERROR, "Could not read from ahci drive.\n" );
		return VFS_ERROR_UNKNOWN;
	}

	return VFS_ERROR_NONE;
}

uint8_t *vfs_disk_write( uint64_t drive, uint8_t *buff, size_t size, off_t offset ) {
	return vfs_disk_write_no_cache( drive, buff, size, offset );
}

uint8_t *vfs_disk_write_no_cache( uint64_t drive, uint8_t *buff, size_t size, off_t offset ) {
	return 0;
}

/** Debug functions */

void vfs_dump_mount_points( void ) {
	for(int i = 0; i < mount_points->size; i++ ) {
		vfs_mount_point *mp = (vfs_mount_point *)avs_list_at_index_data( mount_points, i );

		printf( "Mount point %d:    root=%s    type=%s\n", i, mp->root, mp->fs_type );
	}
}

/** File Descriptors */



vfs_fd global_fds[VFS_FD_MAX];

void vfs_fd_setup( void ) {
	// Setup base structures
	memset( &global_fds, 0, sizeof(vfs_fd) * VFS_FD_MAX );

	for( int i = 0; i < VFS_FD_MAX; i++ ) {
		global_fds[i].id = i;
	}

	// Setup stdin, stdout, stderr
	global_fds[ STDIN_FILENO ].in_use = true;
	strcpy( global_fds[ STDIN_FILENO ].path, "/dev/stdin" );

	global_fds[ STDOUT_FILENO ].in_use = true;
	strcpy( global_fds[ STDOUT_FILENO ].path, "/dev/stdout" );

	global_fds[ STDERR_FILENO ].in_use = true;
	strcpy( global_fds[ STDERR_FILENO ].path, "/dev/stderr" );
}

vfs_fd *vfs_alloc_fd( void ) {
	vfs_fd *fd_free = NULL;

	for( int i = 0; i < VFS_FD_MAX; i++ ) {
		if( global_fds[i].in_use == false ) {
			fd_free = &global_fds[i];
		}
	}

	return fd_free;
}

void vfs_free_fd( int fd ) {
	if( fd > 0 ) {
		if( fd < VFS_FD_MAX ) {
			global_fds[fd].in_use = false;
		} else {
			klog( LOG_ERROR, "fd greater than max vfs fds: %d", fd );
		}
	} else {
		klog( LOG_ERROR, "vfs fd less than 0: %d", fd );
	}
}

vfs_fd *vfs_get_fd_data( int fd ) {
	if( fd >= VFS_FD_MAX ) {
		return NULL;
	}

	if( fd < 0 ) {
		return NULL;
	}

	return &global_fds[fd];
}