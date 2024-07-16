#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "afs.h"
#include <sys/stat.h>
#include <dirent.h>


#define afsfile "/usr/local/osdev/versions/vi/afs.img"

typedef struct {
	char 	name[25];
	int		type;
	char	loc[256];
	char	original_loc[256];
} drive_item;

/* drive_item items[] = {
	{
		.name = "bin",
		.type = AFS_BLOCK_TYPE_DIRECTORY,
		.loc = "/"
	},
	{
		.name = "lib",
		.type = AFS_BLOCK_TYPE_DIRECTORY,
		.loc = "/"
	},
	{
		.name = "etc",
		.type = AFS_BLOCK_TYPE_DIRECTORY,
		.loc = "/"
	},
	{
		.name = "libmyshare.so",
		.type = AFS_BLOCK_TYPE_FILE,
		.loc = "/lib/",
		.original_loc = "/usr/local/osdev/versions/v/support/libtest/libmyshare.so"
	},
	{
		.name = "picard_history.txt",
		.type = AFS_BLOCK_TYPE_FILE,
		.loc = "/lib/",
		.original_loc = "/usr/local/osdev/versions/v/support/afsdrive/picardhistory.txt"
	},
	{
		.name = "magic_key",
		.type = AFS_BLOCK_TYPE_FILE,
		.loc = "/etc/",
		.original_loc = "/usr/local/osdev/versions/v/support/afsdrive/magic_key"
	},
	{
		.name = "cat",
		.type = AFS_BLOCK_TYPE_FILE,
		.loc = "/bin/",
		.original_loc = "/usr/local/osdev/versions/v/support/apptest/myapp"
	},
	{
		.name = "vera.sfn",
		.type = AFS_BLOCK_TYPE_FILE,
		.loc = "/etc/",
		.original_loc = "/usr/local/osdev/versions/v/support/afsdrive/vera.sfn"
	}
}; */

#define MAX_DRIVE_ITEMS 256

uint32_t drive_size_in_bytes = 1024 * 1024 * 2;
char str_test[] = "Hello, world!";
drive_item items[ MAX_DRIVE_ITEMS ];
int	drive_item_count = 0;

afs_drive * bs_drive;
afs_string_table * bs_string_table;
afs_block_directory *bs_root_dir;
afs_block_directory *bs_bin_dir;

vv_file_internal file_system;

void add_files_in_dir( char *dir_name, char *full_path );

void add_files_in_dir( char *dir_name, char *full_path ) {
	printf( "add_files_in_dir( \"%s\", \"%s\" );\n", dir_name, full_path );

	//printf( "BBBBBB" );

	DIR *d;
	struct dirent *dir;
	char loc_start[] = "/usr/local/osdev/versions/vi/os_root";
	char loc_full[256];

	char location_fix[25];
	memset( location_fix, 0, 25 );

	strcat( location_fix, "/" );
	strcat( location_fix, dir_name );
	strcat( location_fix, "/" );

	//printf( "AAAAAAA" );

	d = opendir( full_path );

	if(d) {
		//printf( "....." );
		while( (dir=readdir(d)) != NULL ) {
			printf( "found %s", dir->d_name );
			memset( loc_full, 0, 256 );

			if( strcmp( dir->d_name, "." ) == 0 ) {
				// Skip and continue
				continue;
			}

			if( strcmp( dir->d_name, ".." ) == 0 ) {
				// Skip and continue
				continue;
			}

			strcat( loc_full, full_path );
			strcat( loc_full, "/" );
			strcat( loc_full, dir->d_name );

			if( dir->d_type == DT_REG ) {
				strcpy( items[drive_item_count].loc, location_fix );
				strcpy( items[drive_item_count].name, dir->d_name );
				strcpy( items[drive_item_count].original_loc, loc_full );
				items[drive_item_count].type = AFS_BLOCK_TYPE_FILE;

				drive_item_count++;
			}
		}

		closedir(d);
	} else {
		printf( "d is empty.\n" );
		return;
	}
}

int main( void ) {
	uint8_t * buff = malloc( drive_size_in_bytes );

	// Step 1 -- Format the drive

	FILE *fp = fopen( afsfile, "r+" );

	bootstrap_format( buff, drive_size_in_bytes );

	afs_block_directory *dir_bin = bootstrap_mkdir( buff, bs_root_dir, "bin" );
	afs_block_directory *dir_lib = bootstrap_mkdir( buff, bs_root_dir, "lib" );
	afs_block_directory *dir_etc = bootstrap_mkdir( buff, bs_root_dir, "etc" );
	afs_block_directory *dir_font = bootstrap_mkdir( buff, bs_root_dir, "font" );
	afs_block_directory *dir_home = bootstrap_mkdir( buff, bs_root_dir, "home" );

	DIR *d;
	struct dirent *dir;
	char loc_start[] = "/usr/local/osdev/versions/vi/os_root";
	char loc_full[256];

	d = opendir( loc_start );

	if(d) {
		while( (dir=readdir(d)) != NULL ) {
			memset( loc_full, 0, 256 );

			if( strcmp( dir->d_name, "." ) == 0 ) {
				// Skip and continue
				continue;
			}

			if( strcmp( dir->d_name, ".." ) == 0 ) {
				// Skip and continue
				continue;
			}

			DIR *in_d;
			struct dirent *in_dirent;
			strcat( loc_full, loc_start );
			strcat( loc_full, "/" );
			strcat( loc_full, dir->d_name );

			printf( "found %s\n", dir->d_name );

			if( dir->d_type == DT_DIR ) {
				add_files_in_dir( dir->d_name, loc_full );
			}
		}

		closedir(d);
	} else {
		printf( "d is empty.\n" );
		return;
	}

	for( int i = 0; i < drive_item_count; i++ ) {
		afs_block_directory *dir_to_use = NULL;

		if( strstr( items[i].loc, "etc" ) ) { dir_to_use = dir_etc; }
		if( strstr( items[i].loc, "lib" ) ) { dir_to_use = dir_lib; }
		if( strstr( items[i].loc, "bin" ) ) { dir_to_use = dir_bin; }
		if( strstr( items[i].loc, "home" ) ) { dir_to_use = dir_home; }
		if( strstr( items[i].loc, "font" ) ) { dir_to_use = dir_font; }

		switch( items[i].type ) {
			case AFS_BLOCK_TYPE_DIRECTORY:
				printf( "Adding directory \"%s%s\"\n", items[i].loc, items[i].name );
				break;
			case AFS_BLOCK_TYPE_FILE:
				printf( "Adding file \"%s%s\" from \"%s\"\n", items[i].loc, items[i].name, items[i].original_loc );

				bootstrap_cp( items[i].original_loc, items[i].name, buff, dir_to_use );
				break;
			default:
				printf( "ERROR in block type of item %d\n", i );
		}
	}

	/*
	// Step 2 -- Add files to root
	
	bootstrap_add_file( buff, bs_root_dir, str_test, strlen( str_test ), "testing.txt" );
	bootstrap_cp( "hi", "log.txt", buff, bs_root_dir );
	
	afs_block_directory *dir_bin = bootstrap_mkdir( buff, bs_root_dir, "bin" );
	bootstrap_add_file( buff, dir_bin, "do something", strlen( "do something" ), "do_a_thing" );
	*/

	// Step 3 -- Save drive contents

	size_t written = fwrite( buff, drive_size_in_bytes, 1, fp );

	printf( "ast->nf %d\n", bs_string_table->next_free );
	printf( "Written: %d\n", written );

	for( int i = 0; i < bs_drive->next_free; i++ ) {
		if( *(buff + i) != 0 ) {
			//printf( "%X: %02X\n", i,  *(buff + i) );
		}
	}

	printf( "\n" );
	

	fclose( fp );

	
	// Step 4 -- Test OS routines and read logic

	file_system.drive = buff;
	file_system.root_dir = bs_root_dir;
	file_system.string_table = bs_string_table;
	file_system.next_fd = 0;

	//afs_disply_diagnostic_data( file_system.drive );

	/* vv_file *f = afs_fopen( &file_system, "testing.txt", "r" );

	if( f == NULL ) {
		printf( "fopen returned null\n" );
	}
	
	char *file_buff = malloc( f->size + 1);
	memset( file_buff, 0, f->size + 1);
	
	uint32_t num_read = afs_fread( &file_system, file_buff, f->size, 1, f );

	printf( "num_read: %d\n", num_read );
	printf( "data read: \"%s\"\n", file_buff ); */

	printf( "ls /\n" );
	afs_ls( &file_system, "/" );

	printf( "ls /bin\n" );
	afs_ls( &file_system, "/bin" );

	printf( "ls /etc\n" );
	afs_ls( &file_system, "/etc" );

	printf( "ls /lib\n" );
	afs_ls( &file_system, "/lib" );

	printf( "ls /font\n" );
	afs_ls( &file_system, "/font" );

	printf( "ls /home\n" );
	afs_ls( &file_system, "/home" );
	
	return 0;
}	

bool bootstrap_format( uint8_t *buff, uint32_t size ) {
	bs_drive = (afs_drive *)buff;

	bs_drive->version = AFS_VERSION_1;
	bs_drive->size = size;
	bs_drive->name_index = 0;
	bs_drive->next_free = sizeof( afs_drive );

	bs_string_table = (afs_string_table *)(buff + bs_drive->next_free );
	bs_drive->next_free = bs_drive->next_free + sizeof( afs_string_table );
	strcpy( bs_string_table->string[0], "AFS Drive 1" );
	bs_string_table->next_free++;
	strcpy( bs_string_table->string[1], "." );
	bs_string_table->next_free++;
	strcpy( bs_string_table->string[2], ".." );
	bs_string_table->next_free++;

	bs_root_dir = (afs_block_directory *)(buff + bs_drive->next_free );
	bs_drive->root_directory = bs_drive->next_free;
	bs_root_dir->type = AFS_BLOCK_TYPE_DIRECTORY;
	bs_root_dir->name_index = 0;
	bs_root_dir->next_index = 0;
	memset( bs_root_dir->index, 0, sizeof( afs_index ) * 256 );
	bs_drive->next_free = bs_drive->next_free + sizeof( afs_block_directory );
}

/**
 * @brief Adds a given file to the given directory
 * 
 * @param drive_buff Pointer to the buffer containing the drive
 * @param dir Pointer to the directory to put the file in
 * @param file_buff Pointer to the buffer containing the file
 * @param size Size of the file
 * @param name Name of the file
 * 
 * @return true File adding was successful
 * @return false File adding failed
 */
bool bootstrap_add_file( uint8_t *drive_buff, afs_block_directory *dir, uint8_t *file_buff, uint32_t size, char * name ) {
	afs_file * f = (afs_file *)(drive_buff + bs_drive->next_free);
	f->name_index = bootstrap_add_name_to_string_table( name );
	f->block_size = size + 1024;
	f->file_size = size;
	f->type = AFS_BLOCK_TYPE_FILE;

	dir->index[ dir->next_index ].start = bs_drive->next_free;
	dir->index[ dir->next_index ].name_index = f->name_index;
	dir->next_index++;

	bs_drive->next_free = bs_drive->next_free + sizeof( afs_file );
	
	memcpy( (drive_buff + bs_drive->next_free), file_buff, size );

	bs_drive->next_free = bs_drive->next_free + f->file_size + 1024;

	return true;
}

afs_block_directory* bootstrap_mkdir( uint8_t *drive_buff, afs_block_directory *parent, char *name ) {
	afs_block_directory *d = (afs_block_directory *)(drive_buff + bs_drive->next_free);
	d->name_index = bootstrap_add_name_to_string_table( name );
	d->next_index = 0;
	d->type = AFS_BLOCK_TYPE_DIRECTORY;


	parent->index[ parent->next_index ].name_index = d->name_index;
	parent->index[ parent->next_index ].start = bs_drive->next_free;
	parent->next_index++;

	bs_drive->next_free = bs_drive->next_free + sizeof( afs_block_directory );

	return d;
}

/**
 * @brief Copy a file into the given directory
 * 
 * @param name_old File name (including path) of the file to copy into the structure
 * @param name_new New file name to use
 * @param drive_buff Pointer to the buffer containing the dirve
 * @param dir Pointer to the directory to add the file to
 * 
 * @return true Copying the file was successful
 * @return false Copying the file failed
 */
bool bootstrap_cp( char *name_old, char *name_new, uint8_t *drive_buff, afs_block_directory *dir ) {
	FILE *fp = fopen( name_old, "r" );

	struct stat sta;
	stat( name_old, &sta );
	uint32_t file_size = sta.st_size;

	printf( "size: %d\n", file_size );

	uint8_t * buff = malloc( file_size );

	uint32_t read_size = fread( buff, file_size, 1, fp );
	printf( "read_size: %d\n", read_size );

	bootstrap_add_file( drive_buff, dir, buff, file_size, name_new );

	fclose( fp );
}

/**
 * @brief Add name to the string table
 * 
 * @param name Text string to add
 * 
 * @return uint32_t Index that the name was added at
 */
uint32_t bootstrap_add_name_to_string_table( char * name ) {
	strcpy( bs_string_table->string[ bs_string_table->next_free ], name );
	bs_string_table->next_free++;

	return bs_string_table->next_free - 1;
}


/**************************************************************/
/*                                                            */
/*                                                            */
/* SYNC EVERYTHING BELOW THIS COMMENT BLOCK WITH KERNEL       */
/*                                                            */
/*                                                            */
/**************************************************************/

/**
 * @brief Opens a file, returning FP
 * 
 * @param fs Filesystem object
 * @param filename Name of the file
 * @param mode Mode (libc like): r, w, r+
 * 
 * @return vv_file* Pointer to the file struct
 */
vv_file * afs_fopen( vv_file_internal *fs, const char * filename, const char * mode ) {
	afs_file *f = afs_get_file( fs, filename );
	vv_file *fp;

	// Bail if not found
	if( f == NULL ) {
		return NULL;
	}

	// Use the next available fp
	if( fs->next_fd == MAX_FD - 1 ) {
		return NULL;
	}

	fp = &fs->fd[fs->next_fd];
	fp->fd = fs->next_fd;
	fp->base = (uint8_t *)(f);
	fp->base = fp->base + sizeof(afs_file);
	fp->size = f->file_size;
	fp->position = 0;
	fp->dirty = false;
	fp->name = fs->string_table->string[ f->name_index ];

	//dump_afs_file( fs, f );

	fs->next_fd++;

	return fp;
}

/**
 * @brief Reads data from the given stream
 * 
 * @param fs pointer to the file system object
 * @param ptr pointer to buffer to put read data into
 * @param size number of bytes of each element to read
 * @param nmemb number of elements
 * @param fp pointer to the file pointer object
 * 
 * @return uint32_t number of members read
 */
uint32_t afs_fread( vv_file_internal *fs, void *ptr, uint32_t size, uint32_t nmemb, vv_file *fp ) {
	uint32_t num_read = 0;

	for( int i = 0; i < nmemb; i++ ) {
		if( fp->position + size > fp->size ) {
			i = nmemb + 1;
		}

		memcpy( (ptr + (size * i)), ((uint8_t *)fp->base + fp->position), size );

		fp->position = fp->position + size;
		num_read++;
	}
	
	return num_read;
}

/**
 * @brief Tests if the file exists
 * 
 * @param fs Filesystem object
 * @param filename Name of the file
 * @return bool true if the file exists, otherwise false
 */
uint32_t afs_exists( vv_file_internal *fs, const char * filename ) {
	return afs_get_file( fs, filename ) ? true : false;
}

/**
 * @brief Tests if the file exists
 * 
 * @param fs Filesystem object
 * @param filename Name of the file
 * @return uint32_t byte location of the file's afs_file if found, otherwise 0
 */
uint32_t afs_get_file_location( vv_file_internal *fs, const char *filename ) {
	afs_block_directory *d = fs->root_dir;
	uint32_t loc = 0;

	for( int i = 0; i < d->next_index; i++ ) {
		if( strcmp( fs->string_table->string[ d->index[i].name_index ], filename ) == 0 ) {
			loc = d->index[i].start;
			i = 1000000;
		}
	}

	return loc;
}

/**
 * @brief Returns bytes of file in the given dir
 * 
 * @param fs Filesystem object
 * @param dir Directory object
 * @param filename Name of the file
 * @return uint32_t byte location of the file's afs_file if found, otherwise 0
 */
uint32_t afs_get_file_location_in_dir( vv_file_internal *fs, afs_block_directory *d, const char *filename ) {
	uint32_t loc = 0;

	for( int i = 0; i < d->next_index; i++ ) {
		if( strcmp( fs->string_table->string[ d->index[i].name_index ], filename ) == 0 ) {
			loc = d->index[i].start;
			i = 1000000;
		}
	}

	return loc;
}

/**
 * @brief Returns an afs_file object for the given filename
 * 
 * @param fs Pointer to the filesystem object
 * @param filename Name of the file to retrieve
 * @return afs_file* Pointer to the given file struct
 */
afs_file* afs_get_file( vv_file_internal *fs, const char *filename ) {
	afs_file *file = NULL;
	uint32_t loc;

	loc = afs_get_file_location( fs, filename );

	if( loc == 0 ) {
		return NULL;
	}

	file = (afs_file *)( (uint8_t *)fs->drive + loc );

	return file;
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

/**
 * @brief Send list of files to stdout
 * 
 * @param fs Pointer to the file system object
 * @param path Path of directory to list
 */
void afs_ls( vv_file_internal *fs, char *path ) {
	afs_generic_block *block = afs_get_generic_block( fs, path );

	if( block == NULL ) {
		printf( "Directory not found.\n" );
		return;
	}

	if( block->type == AFS_BLOCK_TYPE_DIRECTORY ) {
		afs_block_directory *dir_to_list = (afs_block_directory *)block;

		for( int i = 0; i < dir_to_list->next_index; i++ ) {
			printf( "%s  ", fs->string_table->string[ dir_to_list->index[i].name_index ] );
		}
	} else {
		printf( "Not a directory." );
	}
	
	printf( "\n" );
}

bool afs_mkdir( vv_file_internal *fs, afs_block_directory *d, char *name ) {
	d->index[ d->next_index ].start = fs->drive->next_free;
	d->index[ d->next_index ].name_index = afs_add_string( fs, name );
}

#undef KDEBUG_GET_GENERIC_BLOCK
/**
 * @brief Get the generic block of the item located at filename
 * 
 * @param fs Pointer to the filesystem object
 * @param filename Filename (and path) of the block to get
 * @return afs_generic_block* Generic block, acceptable to cast to given type
 */
afs_generic_block* afs_get_generic_block( vv_file_internal *fs, char *filename ) {
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
		strcpy( full_filename, fs->working_directory );
		
		int wd_len = strlen( fs->working_directory );

		if( full_filename[wd_len] != '/' ) {
			full_filename[wd_len] = '/';
			full_filename[wd_len + 1] = 0;
		}

		strcat( full_filename, filename );
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
		for( int x = 0; x < 256; x++, i++ ) {
			item_name[x] = full_filename[i];

			if( full_filename[i] == '/' ) {
				x = 256;
			}

			if( full_filename[i] == 0 ) {
				x = 256;
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

				result = (afs_generic_block *)((uint8_t *)fs->drive + loc );
				if( result->type == AFS_BLOCK_TYPE_DIRECTORY ) {
					d = (afs_block_directory *)result;
				} 

				#ifdef KDEBUG_GET_GENERIC_BLOCK
				printf( "block type: %d\n", result->type );
				#endif
			}
		}
		
		#ifdef KDEBUG_GET_GENERIC_BLOCK
		printf( "found: %d\n", found );
		#endif
	}

	return result;
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