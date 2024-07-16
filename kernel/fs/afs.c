#include <kernel_common.h>
#include <afs.h>

#undef KDEBUG_AFS_OPEN
/**
 * @brief Opens a file, returning FP
 * 
 * @param fs Filesystem object
 * @param filename Name of the file
 * @param mode Mode (libc like): r, w, r+
 * 
 * @return vv_file* Pointer to the file struct
 */
vv_file * afs_open( vv_file_internal *fs, const char * filename, const char * mode ) {
	afs_file *f_pointer, f;
	vv_file *fp;
	
	memset( &f, 0, sizeof(afs_file) );

	f_pointer = afs_get_file( fs, filename, &f );
	
	// Bail if not found
	if( ! f_pointer ) {
		klog( "afs_open: afs_get_file returned NULL.\n" );
		return NULL;
	}

	// Use the next available fp
	if( fs->next_fd == MAX_FD - 1 ) {
		return NULL;
	}

	#ifdef KDEBUG_AFS_OPEN
	printf( "filename: \"%s\"\n", filename );
	#endif

	uint32_t base = afs_get_file_location( fs, filename );

	#ifdef KDEBUG_AFS_OPEN
	printf( "base: 0x%X\n", base );
	klog( "hit\n" );
	#endif

	fp = &fs->fd[fs->next_fd];
	fp->fd = fs->next_fd;
	fp->base = base + sizeof(afs_file);
	fp->size = f.file_size;
	fp->position = 0;
	fp->dirty = false;
	fp->name = fs->string_table->string[ f.name_index ];

	fs->next_fd++;

	return fp;
}

#undef KDEBUG_AFS_READ
/**
 * @brief Reads data from the given stream
 * 
 * @param fs pointer to the file system object
 * @param ptr pointer to buffer to put read data into
 * @param size number of bytes to read
 * @param fp pointer to the file pointer object
 * 
 * @return uint32_t number of members read
 */
uint32_t afs_read( vv_file_internal *fs, void *ptr, uint32_t size, vv_file *fp ) {
	uint32_t bytes_read = 0;

	#ifdef KDEBUG_AFS_READ
	klog( "fs: 0x%X\n", fs );
	klog( "ptr: 0x%X\n", ptr );
	klog( "size: %d\n", size );
	klog( "fp: 0x%X\n", fp );
	klog( "fp.pos: 0x%X\n", fp->position );
	klog( "fp.base: 0x%X\n", fp->base );
	#endif

	bytes_read = ahci_read_at_byte_offset_512_chunks( fp->base + fp->position, size, (uint8_t *)ptr );

	#ifdef KDEBUG_AFS_READ
	klog( "num bytes read: %d\n", bytes_read );
	#endif 
	
	return bytes_read;
}

/**
 * @brief Tests if the file exists
 * 
 * @param fs Filesystem object
 * @param filename Name of the file
 * @return bool true if the file exists, otherwise false
 */
uint32_t afs_exists( vv_file_internal *fs, const char * filename ) {
	if( afs_get_file_location( fs, filename ) == 0 ) {
		return false;
	}

	return true;
}

#undef KDEBUG_AFS_GET_FILE_LOCATION
/**
 * @brief Tests if the file exists
 * 
 * @param fs Filesystem object
 * @param filename Name of the file
 * @return uint32_t byte location of the file's afs_file if found, otherwise 0
 */
uint32_t afs_get_file_location( vv_file_internal *fs, const char *filename ) {
	afs_block_directory d;
	afs_block_directory *dd;
	uint32_t loc = 0;

	memset( &d, 0, sizeof(afs_block_directory) );
	dd = afs_get_parent_dir( fs, filename, &d );

	if( ! dd ) {
		printf( "afs_get_file_location: parent dir is NULL\n" );
		return 0;
	}

	//printf( "d.next_index = %d\n", d.next_index );

	// BUG: Need to get the filename from the given path (in filename). This is what we need to compare in the next code block.

	char actual_filename[256];
	memset( actual_filename, 0, 256 );

	if( strchr( filename, '/' ) ) {
		char *last_slash = strrchr( filename, '/' );
		strcpy( &actual_filename, (last_slash + 1) );
	} else {
		strcpy( &actual_filename, filename );
	}
	
	#ifdef KDEBUG_AFS_GET_FILE_LOCATION
	printf( "filename: %s\n", filename );
	printf( "afs_get_file_location: actual file name = %s\n", actual_filename );
	printf( "dd name: \"%s\"\n", fs->string_table->string[ dd->name_index ] );
	printf( "dd->next_index: %d\n", dd->next_index );
	#endif

	for( int i = 0; i < dd->next_index; i++ ) {
		if( strcmp( fs->string_table->string[ dd->index[i].name_index ], actual_filename ) == 0 ) {
			loc = dd->index[i].start;
			i = 1000000;
			
			#ifdef KDEBUG_AFS_GET_FILE_LOCATION
			printf( "hit\n" );
			#endif
		}
	}

	#ifdef KDEBUG_AFS_GET_FILE_LOCATION
	printf( "afs_get_file_location: loc = 0x%X\n", loc );
	#endif

	return loc;
}

#undef KDEBUG_AFS_GET_FILE
/**
 * @brief Returns an afs_file object for the given filename
 * 
 * @param fs Pointer to the filesystem object
 * @param filename Name of the file to retrieve
 * @return afs_file* Pointer to the given file struct
 */
afs_file* afs_get_file( vv_file_internal *fs, const char *filename, afs_file *f ) {
	afs_file *file = NULL;
	afs_generic_block gen_block;
	uint32_t loc;

	file = (afs_file *)afs_get_generic_block( fs, filename, &gen_block );

	if( ! file ) {
		#ifdef KDEBUG_AFS_GET_FILE
		klog( "afs_get_file: file is NULL for %s\n", filename );
		dump_stack_trace();
		#endif

		return NULL;
	}

	f->block_size = file->block_size;
	f->file_size = file->file_size;
	f->name_index = file->name_index;
	f->type = file->type;

	return f;
}

/**
 * @brief Dump drive diagnostic data
 * 
 * @param buff Pointer to the buffer containing the drive
 */
void afs_disply_diagnostic_data( uint8_t * buff ) {

	// First dump the drive info

	afs_drive * drive = (afs_drive *)buff;

	printf( "drive.version: %d\n", drive->version );
	printf( "drive.size: %d\n", drive->size );
	printf( "drive.name_index: %d\n", drive->name_index );
	printf( "drive.root_directory: %d\n", drive->root_directory );
	printf( "drive.next_free: %d\n", drive->next_free );
	printf( "\n" );

	// Dump string index

	afs_string_table *st = (afs_string_table *)(buff + sizeof(afs_drive));

	for( int i = 0; i < st->next_free; i++ ) {
		printf( "string_table[%d] = \"%s\"\n", i, st->string[i] );
	}
	printf( "\n" );

	// Dump dir struct

	afs_block_directory * d = (afs_block_directory *)(buff + drive->root_directory);

	printf( "dir.next_index = %d\n", d->next_index );
	printf( "dir.name_index = %d\n", d->name_index );
	
	for( int i = 0; i < d->next_index; i++ ) {
		printf( "dir.index[%d].start = 0x%X\n", i, d->index[i].start );
		printf( "dir.index[%d].name_index = \"%s\"\n", i, st->string[d->index[i].name_index] );
	}
	printf( "\n" );

	// Dump file stucts

	for( int i = 0; i < d->next_index; i++ ) {
		afs_file *f = (afs_file *)(buff + d->index[i].start );

		printf( "f.block_size: %d\n", f->block_size );
		printf( "f.file_size: %d\n", f->file_size );
		printf( "f.name_index = \"%s\"\n", st->string[f->name_index]);
		printf( "\n" );

		printf( "%s\n\n", (char *)((char *)f + sizeof( afs_file )) );
	}
}

void dump_afs_file( vv_file_internal *fs, afs_file *f ) {
	afs_string_table *st = fs->string_table;

	printf( "f.block_size: %d\n", f->block_size );
	printf( "f.file_size: %d\n", f->file_size );
	printf( "f.name_index = \"%s\"\n", st->string[f->name_index]);
	printf( "\n" );

	printf( "%s\n\n", (char *)((char *)f + sizeof( afs_file )) );
}

#undef KDEBUG_GET_GENERIC_BLOCK
/**
 * @brief Get the generic block of the item located at filename
 * 
 * @param fs Pointer to the filesystem object
 * @param filename Filename (and path) of the block to get
 * @return afs_generic_block* Generic block, acceptable to cast to given type
 */
afs_generic_block* afs_get_generic_block( vv_file_internal *fs, char *filename, afs_generic_block *result_block ) {
	char full_filename[256];
	int i = 0;
	char item_name[256];
	afs_block_directory *d = fs->root_dir;
	bool keep_going = true;
	bool found = false;
	afs_generic_block *result = NULL;

	memset( full_filename, 0, 256 );

	// If first char isn't /, then it's a relative path
		// pre-pend with working dir
	
	if( filename[0] != '/' ) {
		// Handle an empty path at root
		if( filename[0] == 0 ) {
			strcpy( full_filename, fs->working_directory );
		} else {
			strcpy( full_filename, fs->working_directory );
		
			int wd_len = strlen( fs->working_directory );

			if( full_filename[wd_len - 1] != '/' ) {
				full_filename[wd_len] = '/';
				full_filename[wd_len + 1] = 0;
			}

			strcat( full_filename, filename );
		}
		
	} else {
		strcpy( full_filename, filename );
	}

	#ifdef KDEBUG_GET_GENERIC_BLOCK
	printf( "full_filename: \"%s\"\n", full_filename );
	#endif

	// If first char is /, then it's an absolute path
		// iterate over each /
		// result is final iteration
	
	while( full_filename[i] != 0 && keep_going ) {
		memset( item_name, 0, 256 );

		// Copy the current element into item_name
		// HERE THERE BE EVIL
		for( int x = 0; x < 256; x++, i++ ) {
			if( i < 1 ) {
				item_name[x] = full_filename[i];
			}
			
			if( full_filename[i] == '/' ) {
				x = 256;
			}

			if( full_filename[i] == 0 ) {
				x = 256;
			}

			if( i >= 1 ) {
				item_name[x] = full_filename[i];
			}
		}

		#ifdef KDEBUG_GET_GENERIC_BLOCK
		printf( "item_name: \"%s\"\n", item_name );
		#endif

		// If item name is /, then continue
		if( strcmp(item_name, "/") == 0 ) {
			d = fs->root_dir;

			// If this is it, then just return the root directory
			if( strcmp(full_filename, "/") == 0 ) {
				return d;
			}

			found = true;
		} else {
			// Test if item_name exists
			uint32_t loc = afs_exists_in_dir( fs, d, item_name );

			if( loc == 0 ) {
				keep_going = false;
				found = false;
			} else {
				found = true;
				
				#ifdef KDEBUG_GET_GENERIC_BLOCK
				printf( "loc: 0x%X\n", loc );
				#endif

				// TODO: don't use a hardcoded overflow to make sure that we encompas all block types' sizes
				if( ! ahci_read_at_byte_offset( loc, sizeof( afs_generic_block), result_block ) ) {
					printf( "FS GGB: Read of generic block location failed.\n" );
					return;
				}

				if( result_block->type == AFS_BLOCK_TYPE_DIRECTORY ) {
					d = (afs_block_directory *)result_block;
				} 

				#ifdef KDEBUG_GET_GENERIC_BLOCK
				printf( "block type: %d\n", result_block->type );
				#endif
			}
		}
		
		#ifdef KDEBUG_GET_GENERIC_BLOCK
		printf( "found: %d\n", found );
		#endif
	}

	if( found ) {
		result = result_block;
	}

	#ifdef KDEBUG_GET_GENERIC_BLOCK
	dump_stack_trace();
	#endif

	return result;
}

#undef KDEBUG_AFS_LS
/**
 * @brief Send list of files to stdout
 * 
 * @param fs Pointer to the file system object
 * @param path Path of directory to list
 */
void afs_ls( vv_file_internal *fs, char *path ) {
	afs_generic_block _block;
	afs_generic_block *block = NULL;
	
	memset( &_block, 0, sizeof(afs_generic_block) );

	block = afs_get_generic_block( fs, path, &_block );

	if( block == NULL ) {
		printf( "Directory not found.\n" );
		return;
	}

	if( block->type == AFS_BLOCK_TYPE_DIRECTORY ) {
		afs_block_directory *dir_to_list = (afs_block_directory *)block;

		#ifdef KDEBUG_AFS_LS
		printf( "dir.next_index = %d\n", dir_to_list->next_index );
		printf( "dir.name_index = %d\n", dir_to_list->name_index );
	
		for( int i = 0; i < dir_to_list->next_index; i++ ) {
			printf( "dir.index[%d].start = 0x%X\n", i, dir_to_list->index[i].start );
			printf( "dir.index[%d].name_index = %d\n", i, dir_to_list->index[i].name_index );
			char * s = fs->string_table->string[dir_to_list->index[i].name_index];
			printf( "dir.index[%d].name  = \"%s\"\n", i, s );
		}
		printf( "\n" );
		#endif

		for( int i = 0; i < dir_to_list->next_index; i++ ) {
			printf( "%s  ", fs->string_table->string[ dir_to_list->index[i].name_index ] );
		}
	} else {
		printf( "Not a directory." );
	}
	
	printf( "\n" );
}

/**
 * @brief Add name to the string table
 * 
 * @param name Text string to add
 * 
 * @return uint32_t Index that the name was added at
 */
uint32_t afs_add_string( vv_file_internal *fs, char * name ) {
	strcpy( fs->string_table->string[ fs->string_table->next_free ], name );
	fs->string_table->next_free++;

	return fs->string_table->next_free - 1;
}

uint32_t afs_exists_in_dir( vv_file_internal *fs, afs_block_directory *d, char *name ) {
	uint32_t result = 0;

	for( int i = 0; i < d->next_index; i++ ) {
		if( strcmp( fs->string_table->string[ d->index[i].name_index ], name ) == 0 ) {
			result = d->index[i].start;
		}
	}

	return result;
}

#undef KDEBUG_AFS_GET_PARENT_DIR
/**
 * @brief Returns the parent directory of the given file
 * 
 * @param fs vv_file_internal fs object
 * @param name Full pathname + file of the given file. Assumes starting at root (ie: /)
 * @param d directory object to use
 * @return afs_block_directory* returned d
 */
afs_block_directory * afs_get_parent_dir( vv_file_internal *fs, char *name, afs_block_directory *d ) {
	afs_generic_block gen_block;
	afs_block_directory *dir_returned;

	// handle root dir case(s)
	if( strchr(name, '/') == NULL ) {
		#ifdef KDEBUG_AFS_GET_PARENT_DIR
		printf( "afs_get_parent_dir: did not find /, returning root_dir\n" );
		#endif

		return fs->root_dir;
	}

	if( strcmp( name, "/" ) == 0 ) {
		#ifdef KDEBUG_AFS_GET_PARENT_DIR
		printf( "/ is only char, returning root_dir\n" );
		#endif

		return fs->root_dir;
	}

	/* 
	Given input in the format of:
	/element/element-n/element-n+1

	Then remove /element-n+1
	*/

	char path_to_modify[256];
	memset( path_to_modify, 0, 256 );
	strcpy( path_to_modify, name );

	for( int i = strlen(name); i > 0; i-- ) {
		if( path_to_modify[i - 1] == '/' ) {
			path_to_modify[i - 1] = 0;
			i = 0;
		}
	}

	#ifdef KDEBUG_AFS_GET_PARENT_DIR
	printf( "path_to_modify: \"%s\"\n", path_to_modify );
	#endif

	if( strcmp( path_to_modify, "" ) == 0 ) {
		#ifdef KDEBUG_AFS_GET_PARENT_DIR
		printf( "path_to_modify is empty, returning root_dir\n" );
		#endif

		dir_returned = fs->root_dir;
	} else {
		if( ! afs_get_generic_block( fs, path_to_modify, &gen_block ) ) {
			#ifdef KDEBUG_AFS_GET_PARENT_DIR
			printf( "afs_get_parent_dir: gen block is NULL\n" );
			#endif

			return NULL;
		}

		dir_returned = (afs_block_directory *)&gen_block;
	}

	memcpy( d, dir_returned, sizeof( afs_block_directory ) );

	return d;
}