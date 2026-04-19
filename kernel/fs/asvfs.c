#define ASVFS_DEBUG

#include "asvfs.h"
#include <errno.h>

asvfs_instance asvfs_instance_data;
asvfs_header *asvfs_main_drive;
asvfs_block_meta_data *drive_block_meta_data;
asvfs_block_directory *asvfs_root_dir;
asvfs_inode asvfs_inodes;
asvfs_inode *asvfs_inodes_tail;
anode_id asvfs_id_top;
asvfs_dir_t_entries asvfs_dir_entries_data;


/**
 * @brief Initalize the AFS primatives
 * 
 * @return int ASVFS_ERROR_NONE on success, otherwise ASVFS_ERROR_ on failure
 */
int asvfs_initalize( asvfs_drive_ops *ops ) {
	asvfs_inodes_tail = &asvfs_inodes;
	
	asvfs_instance_data.drive_ops.read_from_disk = ops->read_from_disk;
	asvfs_instance_data.drive_ops.write_to_disk = ops->write_to_disk;

	asvfs_instance_data.header = asvfs_malloc( sizeof(asvfs_header) );
	asvfs_main_drive = asvfs_instance_data.header;

	if( asvfs_instance_data.header == NULL ) {
		return ASVFS_ERROR_MEMORY;
	}

	//klog( LOG_DEBUG, "header: %X", asvfs_instance_data.header );
	asvfs_instance_data.drive_ops.read_from_disk( 0, 0, sizeof(asvfs_header), (uint8_t *)asvfs_instance_data.header );

	// verify header, bail if doesn't match
	if( asvfs_instance_data.header->magic[0] == 'A' && asvfs_instance_data.header->magic[1] == 'F' && asvfs_instance_data.header->magic[2] == 'S' ) {
		//klog( LOG_DEBUG, "Header magic verified" );
	} else {
		klog( LOG_DEBUG, "Header magic failed. Expected AFS. Got: \"%c%c%c\".", 
			asvfs_instance_data.header->magic[0],
			asvfs_instance_data.header->magic[1],
			asvfs_instance_data.header->magic[2] );
	}

	asvfs_inodes_tail->block_id = asvfs_instance_data.header->root_directory;
	asvfs_inodes_tail->next = NULL;

	asvfs_root_dir = asvfs_malloc( sizeof(asvfs_block_directory) );

	if( asvfs_root_dir == NULL ) {
		return ASVFS_ERROR_MEMORY;
	}

	int rb_err = asvfs_read_block(asvfs_instance_data.header->root_directory, sizeof(asvfs_block_directory), (char *)asvfs_root_dir);
	/* if( rb_err != sizeof(asvfs_block_directory) ) {
		asvfs_log( "Init: block read of root dir failed. err: %d\n", rb_err );
	} */

	uint32_t length_of_block_meta = asvfs_instance_data.header->block_count * sizeof(asvfs_block_meta_data);
	drive_block_meta_data = asvfs_malloc( length_of_block_meta );

	if( drive_block_meta_data == NULL ) {
		return ASVFS_ERROR_MEMORY;
	}

	asvfs_drive_read( 1, sizeof(asvfs_header), length_of_block_meta, (uint8_t *)drive_block_meta_data );

	#ifdef ASVFS_DEBUG
/* 	asvfs_log( "asvfs_header:\n" );
	asvfs_log( "    magic: \"%c%c%c%c\"\n", asvfs_instance_data.header->magic[0], asvfs_instance_data.header->magic[1], asvfs_instance_data.header->magic[2], asvfs_instance_data.header->magic[3]);
	asvfs_log( "    version: %d\n", asvfs_instance_data.header->version );
	asvfs_log( "    size: %ld\n", asvfs_instance_data.header->size );
	asvfs_log( "    block_size: %d\n", asvfs_instance_data.header->block_size );
	asvfs_log( "    block_count: %d\n", asvfs_instance_data.header->block_count );
	asvfs_log( "    root_directory: %d\n", asvfs_instance_data.header->root_directory );
	asvfs_log( "    next_free: %d\n", asvfs_instance_data.header->next_free ); */
	#endif

	return ASVFS_ERROR_NONE;
}

/**
 * @return	Number of entries
 */
int asvfs_readdir(uint32_t block_id, asvfs_dir_t_entries *dir_entries) {
	//asvfs_log( "AFS in readdir\n" );

	asvfs_block_directory *block_dir_data = asvfs_malloc(sizeof(asvfs_block_directory));
	memset(dir_entries, 0, sizeof(asvfs_dir_t_entries));

	int rb_err = asvfs_read_block(block_id, sizeof(asvfs_block_directory), (char *)block_dir_data);

	if(block_dir_data->type != ASVFS_BLOCK_TYPE_DIRECTORY) {
		asvfs_log( "AFS Block type NOT DIR: %d\n", block_dir_data->type );
		return 0;
	}

	// +2 for dirs . and ..
	dir_entries->size = block_dir_data->next_index + 2;

	strcpy(dir_entries->dir_entry[0].name,".");
	dir_entries->dir_entry[0].type = ASVFS_INODE_TYPE_DIR;

	strcpy(dir_entries->dir_entry[1].name,"..");
	dir_entries->dir_entry[1].type = ASVFS_INODE_TYPE_DIR;

	//asvfs_log( "AFS dir index iterations:\n" );
	for( int i = 0; i < block_dir_data->next_index; i++ ) {
		asvfs_block_meta_data block_meta_data;
		asvfs_get_meta_data(block_dir_data->index[i], &block_meta_data);

		switch(block_meta_data.block_type) {
			case ASVFS_BLOCK_TYPE_DIRECTORY:
				dir_entries->dir_entry[i + 2].type = ASVFS_BLOCK_TYPE_DIRECTORY;
				break;
			case ASVFS_BLOCK_TYPE_FILE:
			default:
				dir_entries->dir_entry[i + 2].type = ASVFS_BLOCK_TYPE_FILE;
		}

		strcpy(dir_entries->dir_entry[i + 2].name, block_meta_data.name);
		dir_entries->dir_entry[i + 2].block_id = block_meta_data.id;

		//asvfs_log( "    AFS DIR Entry: Type=%d Name='%s'\n", dir_entries->dir_entry[i + 2].type, dir_entries->dir_entry[i + 2].name);
	}

	free(block_dir_data);

	return 0;
}

/**
 * @brief Returns the header of the current ASVFS instance
 * 
 * @return asvfs_header* 
 */
asvfs_header *asvfs_get_header( void ) {
	return asvfs_instance_data.header;
}

/**
 * @brief Reads meta data for block_id
 * 
 * @param block_id 
 * @param data 
 * @return int error code
 */
int asvfs_get_meta_data(uint32_t block_id, asvfs_block_meta_data *data) {
	uint64_t offset = sizeof(asvfs_header);
	offset = offset + (sizeof(asvfs_block_meta_data) * block_id);
	asvfs_instance_data.drive_ops.read_from_disk( 0, offset, sizeof(asvfs_block_meta_data), (uint8_t*)data );

	return ASVFS_ERROR_NONE;
}

uint32_t asvfs_get_block_id_from_pathname( char *pathname ) {
	uint32_t current_dir_block_id = 0;
	uint32_t path_length = strlen( pathname );

	if( strcmp( pathname, "/" ) == 0 ) {
		return asvfs_main_drive->root_directory;
	}

	// Break up path by seperator
	// Test each element in turn
	// Return last element
	
	bool keep_going = true;
	uint32_t parent_block_id = 0;
	char *c = pathname;
	int element_index = 0;
	int path_index = 0;
	char name[256];
	memset( name, 0, 256 );

	current_dir_block_id = asvfs_main_drive->root_directory;

	do {	
		if( *c != '/' && *c != 0 ) {
			// build the element

			name[element_index] = *c;
			element_index++;
		} else {
			// done building the element, check for it
			//vfs_debugf( "Element: \"%s\"\n", name );

			if( element_index == 0 ) {
				// ignore root dir
			} else {
				current_dir_block_id = asvfs_get_block_meta_from_dir( current_dir_block_id, name );
				if( current_dir_block_id != 0 ) {
					// do it again
					memset( name, 0, 256 );
					element_index = 0;
				} else {
					// coudln't find it, fail overall
					return 0;
				}
			}
		}

		c++;
		path_index++;

		if( path_index > path_length ) {
			keep_going = false;
		}
	} while( keep_going );

	return current_dir_block_id;
}

int asvfs_get_block_meta_from_dir( uint32_t dir_block_id, char *dir_entry_name ) {
	asvfs_dir_t_entries entries;

	int result = asvfs_readdir( dir_block_id, &entries );

	if( result != 0 ) {
		return 0;
	}

	for( int i = 0; i < entries.size; i++ ) {
		if( strcmp(entries.dir_entry[i].name, dir_entry_name) == 0 ) {
			return entries.dir_entry[i].block_id;
		}
	}

	return 0;
}

int asvfs_getattr( char *pathname, struct stat *stbuff ) {
	asvfs_block_meta_data meta_data;

	uint32_t block_id = asvfs_get_block_id_from_pathname( pathname );

	if( block_id == 0 ) {
		return -ENOENT;
	}

	memset( &meta_data, 0, sizeof(asvfs_block_meta_data) );
	if( asvfs_get_meta_data(block_id, &meta_data) != ASVFS_ERROR_NONE ) {
		return ASVFS_ERROR_UNKNOWN;
	}

	stbuff->st_blksize = 4096;
	stbuff->st_mode = meta_data.permissions = 0666;
	stbuff->st_size = meta_data.file_size;
	stbuff->st_blocks = (meta_data.num_blocks * stbuff->st_blksize) / 512;
	stbuff->st_ino = block_id;
	stbuff->st_atime = stbuff->st_ctime = stbuff->st_mtime = 1767268800;
	//stbuff->st_atimensec = stbuff->st_ctimensec = stbuff->st_mtimensec = 0;

	//klog( LOG_INFO, "pn=%s  md.name=%s  st_size=%d", pathname, meta_data.name, meta_data.file_size );
	//kdebug_peek_at( &meta_data );

	switch( meta_data.block_type ) {
		case ASVFS_BLOCK_TYPE_DIRECTORY:
			stbuff->st_mode = 0775 | S_IFDIR;
			stbuff->st_size = 4096;
			stbuff->st_nlink = 2;
			break;
		case ASVFS_BLOCK_TYPE_FILE:
		default:
			stbuff->st_mode = meta_data.permissions | S_IFREG;
			stbuff->st_nlink = 1;
			break;
	}

	// User ID of 1000 by default
	stbuff->st_uid = 1000;
	stbuff->st_gid = 1000;
	
	return ASVFS_ERROR_NONE;
}

int asvfs_open( char *pathname, int flags, mode_t mode ) {
	uint32_t block_id = asvfs_get_block_id_from_pathname( pathname );

	printf( "ASVFS: O_CREAT? %d from 0x%X\n", flags & O_CREAT, flags );

	if(!block_id) {
		return ASVFS_ERROR_FILE_NOT_FOUND;
	}

	return ASVFS_ERROR_NONE;
}

/**
 * @brief Reads data from given pathname into buf
 * 
 * @param pathname 
 * @param buff 
 * @param offset 
 * @param length 
 * @return int number of bytes read 
 */
int asvfs_read( char *pathname, char *buf, off_t offset, size_t length ) {
	uint32_t block_id = asvfs_get_block_id_from_pathname( pathname );

	if( block_id == 0 ) {
		asvfs_log( "read failed. Couldn't find block_id from pathname: %s\n", pathname );
		return 0;
	}

	uint32_t drive_offset = (block_id * asvfs_instance_data.header->block_size) + offset;

	return asvfs_instance_data.drive_ops.read_from_disk( 0, drive_offset, length, (uint8_t *)buf );
}

int asvfs_read_file_block_id( uint32_t block_id, char *buf, off_t offset, size_t length ) {
	if( block_id == 0 ) {
		klog( LOG_ERROR, "Block ID is 0" );
		return 0;
	}

	uint32_t drive_offset = (block_id * asvfs_instance_data.header->block_size) + offset;

	return asvfs_instance_data.drive_ops.read_from_disk( 0, drive_offset, length, (uint8_t *)buf );
}

/**
 * @brief Read a block into memory
 * 
 * @param block_id 
 * @param size 
 * @param data 
 * @return uint8_t* 
 */
int asvfs_read_block( uint32_t block_id, size_t size, char *data ) {
	//klog( LOG_DEBUG, "block_id=%d size=%X data=%X", block_id, size, data );

	//data = vfs_disk_read( 0, (block_id * drive->block_size), size, data );
	int err = asvfs_instance_data.drive_ops.read_from_disk( 0, block_id * 4096, size, data );

	if( err != ASVFS_ERROR_NONE ) {
		klog( LOG_ERROR, "Error on asvfs_block_read: %d\n", err );
	}

	//asvfs_log( "AFS read block: %d\n", block_id );

	return err;
}

/**
 * @brief Write to a file
 * 
 * @param pathname 
 * @param buff 
 * @param offset 
 * @param size 
 * @return int bytes written
 */
int asvfs_write( char *pathname, char *buff, off_t offset, size_t size ) {
	asvfs_fprintf( asvfs_stderr, "In Write: path=\"%s\" offset=%ld size=%ld\n", pathname, offset, size );

	uint32_t block_id = asvfs_get_block_id_from_pathname( pathname );

	if( block_id == 0 ) {
		asvfs_log( "  Write failed. Couldn't find block_id from pathname: %s\n", pathname );
		return 0;
	}

	return asvfs_write_file_block_id( block_id, buff, offset, size );
}

int asvfs_write_file_block_id( uint32_t block_id, char *buff, off_t offset, size_t size ) {
	off_t drive_offset = (block_id * asvfs_instance_data.header->block_size) + offset;

	asvfs_block_meta_data m;
	asvfs_get_meta_data( block_id, &m );
	
	// Is the new file larger than it was? If so, expand via truncate
	if( m.file_size < (offset + size) ) {
		int res_trunc = asvfs_truncate_file_block_id( block_id, offset + size );

		if( res_trunc != ASVFS_ERROR_NONE ) {
			asvfs_fprintf( asvfs_stderr, "  Truncate error: %d\n", res_trunc );
			return res_trunc;
		}

		m.file_size = offset + size;
	}

	// Do the file write
	int res_write = asvfs_instance_data.drive_ops.write_to_disk( 0, drive_offset, size, buff );

	// Fail write? Stop.
	if( res_write != size ) {
		asvfs_fprintf( asvfs_stderr, "  Write error: %d\n", res_write );
		return res_write;
	}

	/* // Do the meta write
	int res_meta_write = asvfs_write_meta_data( block_id, &m );

	// Fail meta? Roll back write
	if( res_meta_write != ASVFS_ERROR_NONE ) {
		asvfs_fprintf( asvfs_stderr, "  Meta write error: %d\n", res_meta_write );
		return res_meta_write;
	} */

	// Return something useful
	asvfs_fprintf( asvfs_stderr, "Out Write with return of %d\n", res_write );
	return res_write;
}

/**
 * @brief Writes the given amount of data to the provided block_id
 * 
 * @param block_id 
 * @param size 
 * @param data 
 * @return int number of bytes written
 */
int asvfs_write_block( uint32_t block_id, uint32_t size, char *data ) {
	uint32_t offset = block_id * asvfs_main_drive->block_size;
	return asvfs_instance_data.drive_ops.write_to_disk( 0, offset, size, data );
}

/**
 * @brief Writes the provided fs header to disk
 * 
 * @param h pointer to header
 * @return int bytes written
 */
int asvfs_write_header( asvfs_header *h ) {
	return asvfs_instance_data.drive_ops.write_to_disk( 0, 0, sizeof(asvfs_header), (char *)h );
}

/**
 * @brief Writes the provided meta data record to disk
 * 
 * @param block_id 
 * @param meta 
 * @return int number of bytes written
 */
int asvfs_write_meta_data( uint32_t block_id, asvfs_block_meta_data *meta ) {
	off_t offset = sizeof(asvfs_header);
	offset = offset + (sizeof(asvfs_block_meta_data) * block_id);

	return asvfs_instance_data.drive_ops.write_to_disk( 0, offset, sizeof(asvfs_block_meta_data), (char *)meta );
}

/**
 * @brief Splits pathname string into two parts: parent and child.
 * 
 * ie: /home/adam --> parent: /home  child: adam
 * 
 * @param pathname string of pathname to split
 * @param parent 1024 chars for parent path
 * @param child 256 chars for child part
 * @return int error code or 0 on success
 */
int asvfs_get_path_parts( char *pathname, char *parent, char *child ) {
	char name[256];
	int element_index = 0;
    int path_index = 0;
    bool keep_going = true;
    char parent_dir[1024];
    char file_name[256];
    char *c = pathname;
    int path_length = strlen(c);

    memset( parent_dir, 0, 1024 );
    memset( file_name, 0, 256 );
    memset( name, 0, 256 );

	do {	
        if( *c == '/' || *c == 0 || path_index == path_length ) {
			// done building the element, check for it

			if( element_index == 0 ) {
                // root directory condition
				parent_dir[0] = '/';
			} else {
                if( path_index == path_length ) {
                    strcpy( file_name, name );
                    keep_going = false;
                } else {
                    if( strcmp( parent_dir, "/" ) != 0 ) {
                        strcat( parent_dir, "/" );
                    }
                    strcat( parent_dir, name );
                    memset( name, 0, 256 );
                    element_index = 0;
                }
			}
		} else {
            // build the element

			name[element_index] = *c;
			element_index++;
        }

		c++;
		path_index++;

		if( path_index > path_length ) {
			keep_going = false;
		}
	} while( keep_going );

	strcpy( parent, parent_dir );
	strcpy( child, file_name );

	return ASVFS_ERROR_NONE;
}

/**
 * @brief Creates a non-dir, non-symlink file (node)
 * 
 * @param pathname 
 * @param mode 
 * @param rdev 
 * @return int 
 */
int asvfs_mknod( char *pathname, int mode, int rdev ) {
	asvfs_log( "In mknod: pathname='%s'    mode=o%o    rdev=%d\n", pathname, mode, rdev );
	char parent_dir[1024];
	char file_name[256];
	memset( parent_dir, 0, 1024 );
	memset( file_name, 0, 256 );

	int err_path = asvfs_get_path_parts( pathname, parent_dir, file_name );
	if( err_path != ASVFS_ERROR_NONE ) {
		asvfs_log( LOG_DEBUG, "Path unwinding failed for '%s'.\n", pathname );
		return err_path;
	}

	uint32_t parent_block_id = asvfs_get_block_id_from_pathname( parent_dir );
	if( parent_block_id == 0 ) {
		asvfs_log( "mknod parent path not found: '%s'\n", parent_dir );
		return ASVFS_ERROR_PATH_NOT_FOUND;
	}

	asvfs_block_meta_data parent_meta;
	int err_g_parent_meta = asvfs_get_meta_data( parent_block_id, &parent_meta );
	if( err_g_parent_meta != ASVFS_ERROR_NONE ) {
		asvfs_log( "Failed to get meta for parent: %d\n", err_g_parent_meta );
		return err_g_parent_meta;
	}

	if( parent_meta.block_type != ASVFS_BLOCK_TYPE_DIRECTORY ) {
		asvfs_log( "Parent not a directory. id %d    type %d\n", parent_block_id, parent_meta.block_type );
		return ASVFS_ERROR_NOT_A_DIRECTORY;
	}

	uint32_t new_file_block_id = asvfs_instance_data.header->next_free;

	asvfs_block_meta_data nf_meta;
	int err_g_nf_meta = asvfs_get_meta_data( new_file_block_id, &nf_meta );
	if( err_g_nf_meta != ASVFS_ERROR_NONE ) {
		asvfs_log( "Failed to get meta for new file: %d\n", err_g_nf_meta );
		return err_g_nf_meta;
	}

	asvfs_block_directory parent_block;
	asvfs_read_block( parent_block_id, sizeof(asvfs_block_directory), (char *)&parent_block );

	// Setup header
	asvfs_instance_data.header->next_free++;

	// Setup directory
	parent_block.index[ parent_block.next_index++ ] = new_file_block_id;

	// Setup file meta
	nf_meta.in_use = true;
	strcpy( nf_meta.name, file_name );
	nf_meta.block_type = ASVFS_BLOCK_TYPE_FILE;
	nf_meta.permissions = mode;

	asvfs_log( "next block\n" );

	// Write everything to disk
	asvfs_write_block( parent_block_id, sizeof(asvfs_block_directory), (char *)&parent_block );

	asvfs_log( "next data\n" );
	asvfs_write_meta_data( new_file_block_id, &nf_meta );

	asvfs_log( "next hdr\n" );
	asvfs_write_header( asvfs_instance_data.header );

	asvfs_log( "OUT create\n" );

	return ASVFS_ERROR_NONE;
}

/**
 * @brief Creates a file at the given pathname
 * 
 * @param pathname 
 * @param mode 
 * @return int 
 */
int asvfs_create( char *pathname, int mode ) {
	asvfs_log( "IN create: %s\n", pathname );

	return asvfs_mknod( pathname, mode, 0 );
}

/**
 * @brief Creates the given directory
 * 
 * @param pathname 
 * @param mode 
 * @param rdev 
 * @return int 
 */
int asvfs_mkdir( char *pathname, int mode, int rdev ) {
	asvfs_log( "IN mkdir:\n" );

	char parent_dir[1024];
	char new_dir_name[256];
	memset( parent_dir, 0, 1024 );
	memset( new_dir_name, 0, 256 );

	int err_path = asvfs_get_path_parts( pathname, parent_dir, new_dir_name );
	if( err_path != ASVFS_ERROR_NONE ) {
		printf( "Path unwinding failed for '%s'.\n", pathname );
		return err_path;
	}

	uint32_t parent_block_id = asvfs_get_block_id_from_pathname( parent_dir );
	if( parent_block_id == 0 ) {
		return ASVFS_ERROR_PATH_NOT_FOUND;
	}

	asvfs_block_meta_data parent_meta;
	int err_g_parent_meta = asvfs_get_meta_data( parent_block_id, &parent_meta );
	if( err_g_parent_meta != ASVFS_ERROR_NONE ) {
		printf( "Failed to get meta for parent: %d\n", err_g_parent_meta );
		return err_g_parent_meta;
	}

	if( parent_meta.block_type != ASVFS_BLOCK_TYPE_DIRECTORY ) {
		printf( "Parent not a directory. id %d    type %d\n", parent_block_id, parent_meta.block_type );
		return ASVFS_ERROR_NOT_A_DIRECTORY;
	}

	uint32_t new_dir_block_id = asvfs_instance_data.header->next_free;

	asvfs_block_meta_data nd_meta;
	int err_g_nd_meta = asvfs_get_meta_data( new_dir_block_id, &nd_meta );
	if( err_g_nd_meta != ASVFS_ERROR_NONE ) {
		printf( "Failed to get meta for new dir: %d\n", err_g_nd_meta );
		return err_g_nd_meta;
	}

	asvfs_block_directory parent_block;
	asvfs_read_block( parent_block_id, sizeof(asvfs_block_directory), (char *)&parent_block );

	// Setup header
	asvfs_instance_data.header->next_free++;

	// Setup parent directory
	parent_block.index[ parent_block.next_index++ ] = new_dir_block_id;

	// Setup file meta
	nd_meta.in_use = true;
	strcpy( nd_meta.name, new_dir_name );
	nd_meta.block_type = ASVFS_BLOCK_TYPE_DIRECTORY;
	nd_meta.permissions = mode;

	// Setup new directory block
	asvfs_block_directory new_dir_block;
	new_dir_block.next_index = 0;
	new_dir_block.type = ASVFS_BLOCK_TYPE_DIRECTORY;

	// Write everything to disk
	asvfs_write_block( parent_block_id, sizeof(asvfs_block_directory), (char *)&parent_block );
	asvfs_write_block( new_dir_block_id, sizeof(asvfs_block_directory), (char *)&new_dir_block );
	asvfs_write_meta_data( new_dir_block_id, &nd_meta );
	asvfs_write_header( asvfs_instance_data.header );

	return ASVFS_ERROR_NONE;
}

int asvfs_truncate( char *pathname, off_t length ) {
	printf( "IN truncate. path: %s    length: %ld\n", pathname, length );

	uint32_t block_id = asvfs_get_block_id_from_pathname( pathname );
	if( block_id == 0 ) {
		return ASVFS_ERROR_PATH_NOT_FOUND;
	}

	return asvfs_truncate_file_block_id( block_id, length );
}

/**
 * @brief Shrink or expand file pathname to size length
 * 
 * @param pathname 
 * @param length 
 * @return int 
 */
int asvfs_truncate_file_block_id( uint32_t block_id, off_t length ) {
	asvfs_block_meta_data meta;
	int err_meta = asvfs_get_meta_data( block_id, &meta );
	if( err_meta != ASVFS_ERROR_NONE ) {
		printf( "get meta error: %d\n", err_meta );
		return err_meta;
	}

	meta.file_size = length;

	asvfs_header *h = asvfs_get_header();

	// Do we span multiple blocks? if so, account for it
	if( meta.file_size > ASVFS_DEFAULT_BLOCK_SIZE ) {
		uint32_t num_blocks_needed = (meta.file_size / ASVFS_DEFAULT_BLOCK_SIZE) + (meta.file_size % ASVFS_DEFAULT_BLOCK_SIZE ? 1 : 0);
		asvfs_fprintf( asvfs_stderr, "num blocks needed: %d\n", num_blocks_needed );

		// TODO: Don't do this at all, my god terrible
		if( num_blocks_needed > meta.num_blocks ) {
			// We need more blocks! Recreate the file at the end of the drive, assigning the number of blocks needed to it.
			uint32_t new_start_block = h->next_free;
			
			for( int i = 0; i < meta.num_blocks; i++ ) {
				char data[ASVFS_DEFAULT_BLOCK_SIZE];
				memset( data, 0, ASVFS_DEFAULT_BLOCK_SIZE );

				asvfs_read_block( meta.starting_block + i, ASVFS_DEFAULT_BLOCK_SIZE, data );
				asvfs_write_block( new_start_block + i, ASVFS_DEFAULT_BLOCK_SIZE, data );
			}

			// Update the header
			h->next_free = h->next_free + num_blocks_needed;
			asvfs_write_header( h );

			// Update the file's meta
			meta.starting_block = new_start_block;
			meta.num_blocks = num_blocks_needed;
			asvfs_write_meta_data( block_id, &meta );
		}
	}

	asvfs_write_meta_data( block_id, &meta );

	return ASVFS_ERROR_NONE;
}

/**
 * @brief Deletes the given pathname
 * 
 * @param pathname 
 * @return int 
 */
int asvfs_unlink( char *pathname ) {
	asvfs_fprintf( asvfs_stderr, "IN unlink for %s\n", pathname );

	char parent_dir[1024];
	char child_name[256];
	memset( parent_dir, 0, 1024 );
	memset( child_name, 0, 256 );

	int err_path = asvfs_get_path_parts( pathname, parent_dir, child_name );
	if( err_path != ASVFS_ERROR_NONE ) {
		printf( "Path unwinding failed for '%s'.\n", pathname );
		return err_path;
	}

	uint32_t parent_block_id = asvfs_get_block_id_from_pathname( parent_dir );
	if( parent_block_id == 0 ) {
		return ASVFS_ERROR_PATH_NOT_FOUND;
	}

	asvfs_block_directory parent_block;
	memset( &parent_block, 0, sizeof(asvfs_block_directory) );
	asvfs_read_block( parent_block_id, sizeof(asvfs_block_directory), (char *)&parent_block );

	uint32_t child_block_id = asvfs_get_block_id_from_pathname( pathname );
	if( child_block_id == 0 ) {
		return ASVFS_ERROR_PATH_NOT_FOUND;
	}

	asvfs_fprintf( asvfs_stderr, "  unlinking: %d\n", child_block_id );

	for( int x = 0; x < parent_block.next_index; x++ ) {
		asvfs_fprintf( asvfs_stderr, "preu index[%d] = %d\n", x, parent_block.index[x] );
	}

	bool use_next = false;
	for( int i = 0; i < parent_block.next_index; i++ ) {
		if( parent_block.index[i] == child_block_id ) {
			use_next = true;
			asvfs_fprintf( asvfs_stderr, "  found\n" );
		}

		if( use_next ) {
			if( i + 1 >= 256 ) {
				parent_block.index[i] = 0;
			} else if( i == parent_block.next_index - 1 ) {
				parent_block.index[i] = 0;
			} else {
				parent_block.index[i] = parent_block.index[i+1];
			}
		}

		asvfs_fprintf( asvfs_stderr, "  index[%d] = %d\n", i, parent_block.index[i] );
	}

	// Impossible to get to fail?
	if( parent_block.next_index != 0 ) {
		parent_block.next_index--;
	}

	for( int x = 0; x < parent_block.next_index; x++ ) {
		asvfs_fprintf( asvfs_stderr, "postu index[%d] = %d\n", x, parent_block.index[x] );
	}

	asvfs_write_block( parent_block_id, sizeof(asvfs_block_directory), (char *)&parent_block );

	asvfs_fprintf( asvfs_stderr, "OUT unlink\n" );
	return ASVFS_ERROR_NONE;
}

int asvfs_chmod( char *pathname, int mode ) {
	uint32_t  block_id = asvfs_get_block_id_from_pathname( pathname );
	if( block_id == 0 ) {
		return ASVFS_ERROR_PATH_NOT_FOUND;
	}

	asvfs_block_meta_data meta;
	int err_meta = asvfs_get_meta_data( block_id, &meta );
	if( err_meta != ASVFS_ERROR_NONE ) {
		printf( "get meta error: %d\n", err_meta );
		return err_meta;
	}

	meta.permissions = mode;

	asvfs_write_meta_data( block_id, &meta );

	return ASVFS_ERROR_NONE;
}

/**
 * @brief "Renames" a file or directory
 * 
 * @param from Old pathname
 * @param to New pathname
 * @return int 0 on success
 * 
 * Scenarios:
 * 1. Only name change
 * 2. Dir change
 * 2a. Dir change + name change
 */
int asvfs_rename( char *from, char *to ) {
	asvfs_fprintf( asvfs_stderr, "IN asvfs_rename from %s to %s\n", from, to );

	char from_parent_name[1024];
	char from_child_name[256];
	asvfs_get_path_parts( from, from_parent_name, from_child_name );

	char to_parent_name[1024];
	char to_child_name[256];
	asvfs_get_path_parts( to, to_parent_name, to_child_name );

	uint32_t from_path_block_id = asvfs_get_block_id_from_pathname( from_parent_name );
	if( from_path_block_id == 0 ) {
		asvfs_fprintf( asvfs_stderr, "From path not found.\n" );
		return ASVFS_ERROR_PATH_NOT_FOUND;
	}

	uint32_t to_path_block_id = asvfs_get_block_id_from_pathname( to_parent_name );
	if( to_path_block_id == 0 ) {
		asvfs_fprintf( asvfs_stderr, "To path not found.\n" );
		return ASVFS_ERROR_PATH_NOT_FOUND;
	}

	uint32_t from_pathname_block_id = asvfs_get_block_id_from_pathname( from );
	if( from_pathname_block_id == 0 ) {
		asvfs_fprintf( asvfs_stderr, "From pathname not found. Should never fire.\n" );
		return ASVFS_ERROR_PATH_NOT_FOUND;
	}

	if( from_path_block_id == to_path_block_id ) {
		// Yay, easy! Renaming the file in the same directory.

		asvfs_fprintf( asvfs_stderr, "in happy path\n" );

		asvfs_block_meta_data meta;
		int err_meta = asvfs_get_meta_data( from_pathname_block_id, &meta );
		if( err_meta != ASVFS_ERROR_NONE ) {
			asvfs_fprintf( asvfs_stderr, "get meta error: %d\n", err_meta );
			return err_meta;
		}

		strcpy( meta.name, to_child_name );
		asvfs_write_meta_data( from_pathname_block_id, &meta );
	} else {
		// Move file to new parent dir

		asvfs_fprintf( asvfs_stderr, "in hard path\n" );

		// Step 1: Unlink from parent directory
		asvfs_block_meta_data from_parent_meta;
		int err_meta_from_parent = asvfs_get_meta_data( from_path_block_id, &from_parent_meta );
		if( err_meta_from_parent != ASVFS_ERROR_NONE ) {
			asvfs_fprintf( asvfs_stderr, "get meta error: %d\n", err_meta_from_parent );
			return err_meta_from_parent;
		}

		asvfs_unlink( from );

		// Step 2: Add file to new parent directory
		// TODO: Separate out adding a new entry into a dir, remove duplicate code/intention in mknod
		asvfs_block_directory new_parent_dir;
		asvfs_read_block( to_path_block_id, sizeof(asvfs_block_directory), (char *)&new_parent_dir ); // TODO: EC
		// TODO: next_index boundry check
		new_parent_dir.index[ new_parent_dir.next_index++ ] = from_pathname_block_id;
		asvfs_write_block( to_path_block_id, sizeof(asvfs_block_directory), (char *)&new_parent_dir ); // TODO: EC
		
		// with a new name?
		asvfs_fprintf( asvfs_stderr, "fcn: '%s'    tcn: '%s'\n", from_child_name, to_child_name );
		if( strcmp(from_child_name, to_child_name) != 0 ) {
			asvfs_fprintf( asvfs_stderr, "name change.\n" );
			asvfs_block_meta_data meta_file;
			int err_meta_file = asvfs_get_meta_data( from_pathname_block_id, &meta_file );
			if( err_meta_file != ASVFS_ERROR_NONE ) {
				asvfs_fprintf( asvfs_stderr, "get meta error: %d\n", err_meta_file );
				return err_meta_file;
			}

			strcpy( meta_file.name, to_child_name );
			asvfs_write_meta_data( from_pathname_block_id, &meta_file ); // TODO: EC
		}
	}

	

	return 0;
}