#include <kernel_common.h>
#include <fs.h>
#include <afs.h>

vv_file_internal file_system;
afs_drive main_drive;
afs_block_directory root_dir;
afs_string_table string_table;

char name_stdin[] = "stdin";
char name_stdout[] = "stdout";
char name_stderr[] = "srderr";

vv_file *stdin;
vv_file *stdout;
vv_file *stderr;

#define KDEBUG_FS_INIT
/**
 * @brief Initalize the filesystem
 * 
 */
void fs_initalize( void ) {
	strcpy( file_system.working_directory, "/" );
	file_system.drive = &main_drive;
	file_system.root_dir = &root_dir;
	file_system.string_table = &string_table;
	file_system.next_fd = 3;

	stdin = &file_system.fd[ STDIN_FILENO ];
	stdout = &file_system.fd[ STDOUT_FILENO ];
	stderr = &file_system.fd[ STDERR_FILENO ];

	stdin->fd = STDIN_FILENO;
	stdout->fd = STDOUT_FILENO;
	stderr->fd = STDERR_FILENO;

	// Initalize the main drive block
	if( ! ahci_read_at_byte_offset( 0, sizeof(afs_drive), file_system.drive ) ) {
		debugf( "FS: Drive block read failed.\n" );
		return;
	}

	#ifdef KDEBUG_FS_INIT
	debugf( "drive.version: %d\n", file_system.drive->version );
	debugf( "drive.size: %08X\n", file_system.drive->size );
	debugf( "drive.name_index: %d\n", file_system.drive->name_index );
	debugf( "drive.root_directory: %04X\n", file_system.drive->root_directory );
	debugf( "drive.next_free: %04X\n", file_system.drive->next_free );
	debugf( "\n" );
	#endif

	// Load the string table
	if( ! ahci_read_at_byte_offset( sizeof(afs_drive), sizeof(afs_string_table), file_system.string_table ) ) {
		debugf( "FS: String table read failed.\n" );
		return;
	}

	#ifdef KDEBUG_FS_INIT
	for( int i = 0; i < file_system.string_table->next_free; i++ ) {
		debugf( "string_table[%d] = \"%s\" @ 0x%08X\n", i, file_system.string_table->string[i], file_system.string_table->string[i]);
	}
	debugf( "\n" );
	#endif

	// Load the root directory block info
	if( ! ahci_read_at_byte_offset( file_system.drive->root_directory, sizeof( afs_block_directory), file_system.root_dir ) ) {
		debugf( "FS: Root drive read failed.\n" );
		return;
	}

	#ifdef KDEBUG_FS_INIT
	debugf( "dir.next_index = %d\n", file_system.root_dir->next_index );
	debugf( "dir.name_index = %d\n", file_system.root_dir->name_index );
	
	for( int i = 0; i < file_system.root_dir->next_index; i++ ) {
		debugf( "dir.index[%d].start = 0x%X\n", i, file_system.root_dir->index[i].start );
		debugf( "dir.index[%d].name_index = %d\n", i, file_system.root_dir->index[i].name_index );
		char * s = file_system.string_table->string[file_system.root_dir->index[i].name_index];
		debugf( "dir.index[%d].name  = \"%s\"\n", i, s );
	}
	debugf( "\n" );
	#endif
}

void primative_ls( char *path ) {
	afs_ls( &file_system, path );
}

void primative_pwd( void ) {
	debugf( "%s\n", file_system.working_directory );
}

#undef KDEBUG_PRIMATIVE_CAT
/**
 * @brief AFS/Low version of cat for testing
 * 
 * @param pathname path + filename of the file to cat
 */
void primative_cat( char *pathname ) {
	vv_file *fp;

	fp = afs_open( &file_system, pathname, "r" );

	if( ! fp ) {
		printf( "CAT: fp is NULL\n" );
		return;
	}

	#ifdef KDEBUG_PRIMATIVE_CAT
	debugf( "fp.position: 0x%X\n", fp->position );
	debugf( "fp.base: 0x%X\n", fp->base );
	#endif

	uint8_t *data = malloc( fp->size );

	if( afs_read( &file_system, data, fp->size, fp ) == 0 ) {
		printf( "CAT: afs_fread returned 0.\n" );
		return;
	}

	printf( "%s\n", data );

	free( data );
}

/**
 * @brief Get the fs internal object
 * 
 * @return vv_file_internal* Pointer to the file system internal management object
 */
vv_file_internal * get_fs_internal( void ) {
	return &file_system;
}

/**
 * @brief Get the file size of the given FD
 * 
 * @param FD file descriptor to get the size of
 * @return uint32_t size of the file, 0 if empty or invalid
 */
uint32_t get_file_size( int _FD ) {
	return file_system.fd[_FD].size;
}