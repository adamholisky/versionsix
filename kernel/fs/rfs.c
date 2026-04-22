#include <kernel_common.h>
#include <kmemory.h>
#include "vfs.h"
#include "rfs.h"
#include <lib/list.h>

rfs_file_list rfs_files;
rfs_mounted rfs_mounts;

vfs_filesystem *rfs_fs_obj;
vfs_operations rfs_vfs_ops;

avs_list *rfs_file_objs;

/**
 * @brief Initalizes the Ram File System
 * 
 * @return int VFS_ERROR_NONE on success, otherwise VFS_ERROR_ on failure
 */
int rfs_initalize( void ) {
	rfs_file_objs = avs_list_init();

	rfs_vfs_ops.open = rfs_open;
	rfs_vfs_ops.create = rfs_create;
	rfs_vfs_ops.write = rfs_write;
	rfs_vfs_ops.read = rfs_read;
	rfs_vfs_ops.get_dir_list = rfs_dir_list;
	rfs_vfs_ops.getattr = rfs_getattr;

	rfs_fs_obj = vfs_register_fs( "RFS", &rfs_vfs_ops );

	if( rfs_fs_obj == NULL ) {
		klog( LOG_ERROR, "Register FS error for RFS" );
	}

	rfs_files.head = NULL;
	rfs_files.tail = NULL;
	rfs_files.count = 0;

	return VFS_ERROR_NONE;
}

/**
 * @brief Gets the rfs_file object for the given pathname
 * 
 * @param pathname 
 * @return rfs_file* NULL if not found
 */
rfs_file* rfs_lookup_by_pathname( char *pathname ) {
	avs_node *n_f = avs_list_find_data( rfs_file_objs, pathname, rfs_avs_list_compare_file_pathnames );

	if( n_f == NULL ) {
		return NULL;
	}

	return (rfs_file *)n_f->data;
}

int rfs_avs_list_compare_file_pathnames( void *a, void *b ) {
	return strcmp( (char *)a, ((rfs_file *)b)->pathname );
}

/**
 * @brief Opens an RFS file for use
 * 
 * @param id 
 * @return int VFS_ERROR_NONE on success, VFS_ERROR_ on failure
 */
int rfs_open( char *pathname, int flags, mode_t mode) {

	return VFS_ERROR_NONE;
}

/**
 * @brief Returns statistics for the given file
 * 
 * @param id 
 * @param stat 
 * @return int VFS_ERROR_NONE on success, VFS_ERROR_ on failure
 */
int rfs_getattr( char *pathname, struct stat *stbuff ) {
	rfs_file *f = rfs_lookup_by_pathname( pathname );

	if( f == NULL ) {
		klog( LOG_INFO, "RFS file not found: %s", pathname );
		return VFS_ERROR_FILE_NOT_FOUND;
	}

	stbuff->st_size = f->size;
	stbuff->type = f->rfs_file_type;

	return VFS_ERROR_NONE;
}


/**
 * @brief Creates an inode
 * 
 * @param type 
 * @param parent 
 * @param path 
 * @param name 
 * @return int inode id on success, otherwise VFS_ERROR_ on failure
 */
int rfs_create( char *pathname, int mode ) {
	// Allocate a RFS file, fill in details
	rfs_file *f = kmalloc( sizeof(rfs_file) );
	f->size = 0;
	strcpy( f->pathname, pathname );

	switch( mode ) {
		case VFS_INODE_TYPE_DIR:
			f->rfs_file_type = RFS_FILE_TYPE_DIR;
			break;
		case VFS_INODE_TYPE_FILE:
			f->rfs_file_type = RFS_FILE_TYPE_FILE;
			break;
		case VFS_INODE_TYPE_DEVICE:
			f->rfs_file_type = RFS_FILE_TYPE_DEVICE;
			break;
	}

	// If dir, initalize the RFS dir_list
	if( mode == VFS_INODE_TYPE_DIR ) {
		rfs_file_list *d_list = kmalloc( sizeof(rfs_file_list) );
		f->dir_list = (void *)d_list;
		d_list->count = 0;
		d_list->head = NULL;
		d_list->tail = NULL;
	} else {
		f->dir_list = NULL;
	}

	// Insert into parent directory
	/* rfs_file *rfs_parent = rfs_lookup_by_inode_id( parent );
	rfs_file_list *parent_dir = (rfs_file_list *)rfs_parent->dir_list;
	rfs_file_list_el *list_el = vfs_malloc( sizeof(rfs_file_list_el) );
	list_el->next = NULL;
	list_el->file = f;

	if( parent_dir->head == NULL ) {
		parent_dir->head = list_el;
		parent_dir->tail = list_el;
	} else {
		parent_dir->tail->next = list_el;
		parent_dir->tail = list_el;
	}
	parent_dir->count++; */

	// Attach completed file to the master list
	avs_list_append( rfs_file_objs, f );

	return VFS_ERROR_NONE;
}

/**
 * @brief Reads from the given inode
 * 
 * @param id 
 * @param data 
 * @param size 
 * @param offset 
 * @return int size of bytes read, otherwise VFS_ERROR_
 */
int rfs_read( char *pathname, char *buf, off_t offset, size_t length ) {
	rfs_file *f = rfs_lookup_by_pathname( pathname );

	if( f == NULL ) {
		klog( LOG_INFO, "rfs_file not found: %s", pathname );
		return VFS_ERROR_FILE_NOT_FOUND;
	}

	memcpy( buf + (uint8_t)offset, f->data, length );

	return length;
}

/**
 * @brief Writes to an RFS file
 * 
 * @param id 
 * @param data 
 * @param size 
 * @param offset 
 * @return int Bytes written (greater than 0), otherwise VFS_ERROR_ on failure
 */
int rfs_write( char *pathname, char *buff, off_t offset, size_t size ) {
	rfs_file *f = rfs_lookup_by_pathname( pathname );

	if( f == NULL ) {
		klog( LOG_INFO, "rfs_file not found: %s", pathname );
		return VFS_ERROR_FILE_NOT_FOUND;
	}

	// If no size, then it's the first write, so just create the mem and copy the data
	if( f->size == 0 ) {
		f->data = kmalloc( size );
		
		if( f->data == NULL ) {
			klog( LOG_ERROR, "Could not allocate space for file. pathname=%s  size=%d", pathname, size );
			return VFS_ERROR_MEMORY;
		}

		f->size = size;
	}

	// Do a realloc if we don't have enough space
	uint64_t space_to_realloc = 0;

	if( f->size < offset + size ) {
		space_to_realloc = size + offset;
	}

	if( space_to_realloc != 0 ) {
		f->data = krealloc( f->data, space_to_realloc );
	}

	// Copy over the data
	memcpy( (f->data + (uint8_t)offset), buff, size );

	klog( LOG_DEBUG, "Wrote pathname=%s size=%d offset=%d buff=%X data=\"%s\"", pathname, size, offset, buff, buff );

	return size;
}


/**
 * @brief Mounts the RFS fs
 * 
 * @param id 
 * @param data_root 
 * @return int VFS_ERROR_NONE if successful, otherwise VFS_ERROR_
 */
/* int rfs_mount( inode_id id, char *path, uint8_t *data_root ) {
	// Register this mount point
	rfs_mounted *mnt = NULL;

	bool found = false;
	if( rfs_mounts.id == 0 ) {
		mnt = &rfs_mounts;
	} else {
		rfs_mounted *head = &rfs_mounts;

		do {
			if( head->next == NULL ) {
				head->next = vfs_malloc(sizeof(rfs_mounted));
				mnt = head->next;
				found = true;
			}
			head = head->next;
		} while( head != NULL && !found );
	}

	if( mnt == NULL ) {
		return VFS_ERROR_MEMORY;
	}

	mnt->id = id;
	mnt->next = NULL;
	mnt->file_list.tail = NULL;
	strcpy( mnt->path, path );

	vfs_inode *ino = vfs_lookup_inode_ptr_by_id(id);
	mnt->fs_id = ino->fs_id;

	mnt->file_list.count = 1;
	mnt->file_list.head = vfs_malloc( sizeof(rfs_file_list_el) );

	if( mnt->file_list.head == NULL ) {
		return VFS_ERROR_MEMORY;
	}

	mnt->file_list.tail = mnt->file_list.head;

	mnt->file_list.head->file = vfs_malloc( sizeof(rfs_file) );

	if( mnt->file_list.head->file == NULL ) {
		return VFS_ERROR_MEMORY;
	}

	mnt->file_list.head->file->rfs_file_type = RFS_FILE_TYPE_DIR;
	mnt->file_list.head->file->vfs_inode_id = id;
	mnt->file_list.head->file->vfs_parent_inode_id = 0;
	strcpy( mnt->file_list.head->file->name, "/" );

	rfs_file_list *root_dir_list = vfs_malloc( sizeof(rfs_file_list) );

	if( root_dir_list == NULL ) {
		return VFS_ERROR_MEMORY;
	}

	root_dir_list->head = NULL;
	root_dir_list->tail = NULL;
	root_dir_list->count = 0;
	mnt->file_list.head->file->dir_list = (void *)root_dir_list;

	return VFS_ERROR_NONE;
}
 */
/**
 * @brief Gets an RFS file list given the fs_id
 * 
 * @param fs_id 
 * @return rfs_file_list* 
 */
/* rfs_file_list *rfs_get_file_list_by_fs_id( uint8_t fs_id ) {
	rfs_mounted *mnt = NULL;

	if( rfs_mounts.id == 0 ) {
		return NULL;
	}

	rfs_mounted *head = &rfs_mounts;

	do {
		if( head->fs_id == fs_id ) {
			return &head->file_list;
		}

		head = head->next;
	} while( head != NULL );

	return NULL;
}
 */


/**
 * @brief Gets the rfs_file pointer for the given vfs inode id
 * 
 * @param id vfs inode_id
 * @return rfs_file* pointer to rfs_file pointer, NULL on failure
 */
/* rfs_file *rfs_lookup_by_inode_id( inode_id id ) {
	vfs_inode *ino = vfs_lookup_inode_ptr_by_id( id );
	rfs_file_list *rfs_files = rfs_get_file_list_by_fs_id( ino->fs_id );

	rfs_file *f = NULL;
	rfs_file_list_el *head = rfs_files->head;
	bool keep_going = true;
	bool found = false;

	do {
		f = head->file;

		if( f->vfs_inode_id == id ) {
			found = true;
		} else if( head->next == NULL ) {
			keep_going = false;
		} else {
			head = head->next;
		}
	} while( keep_going && !found );

	if( !found ) {
		return NULL;
	}

	return f;
} */




/**
 * @brief Returns a list of files in the given directory
 * 
 * @param id 
 * @param list 
 * @return vfs_directory_list* Directory list on success, NULL on failure
 */
vfs_directory_list *rfs_dir_list( inode_id id, vfs_directory_list *list ) {
	/* rfs_file *dir = rfs_lookup_by_inode_id( id );

	if( dir == NULL ) {
		//vfs_debugf( "rfs_file not found.\n" );
		return NULL;
	}

	if( dir->rfs_file_type != RFS_FILE_TYPE_DIR ) {
		//vfs_debugf( "rfs_file is not a direcotry.\n" );
		return NULL;
	}

	if( dir->dir_list == NULL ) {
		//vfs_debugf( "dir_list in rfs_file is NULL for id %ld.\n", id );
		return NULL;
	}

	rfs_file_list *rfs_list = (rfs_file_list *)dir->dir_list;

	list->count = rfs_list->count;
	list->entry = vfs_malloc( sizeof(vfs_directory_item) * list->count );

	rfs_file_list_el *head = rfs_list->head;

	for( int i = 0; i < list->count; i++ ) {
		strcpy( list->entry[i].name, head->file->name );
		list->entry[i].id = head->file->vfs_inode_id;

		head = head->next;
	} */

	return NULL;
}