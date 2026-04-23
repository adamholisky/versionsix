#include <kernel_common.h>
#include <kshell_app.h>
#include <fs.h>
#include <vfs.h>

#include <asvfs.h>

int main( int argc, char *argv[] ) {
	char path[1024];
	
	memset( path, 0, 1024 );

	if( argc == 2 ) {
		strcpy( path, argv[1] );
	}

	char type_dir[] = "DIR ";
	char type_file[] = "FILE";
	char type_unknown[] = "????";

	if( strcmp( path, "" ) == 0 ) {
		strcpy( path, "/" );
	}

	file_stats st;
	int getattr_err = asvfs_getattr( path, &st );

	vfs_dir *d = vfs_opendir( path );
	if( d == NULL ) {
		printf( "Directory not found.\n" );
		return 1;
	}

	vfs_dirent *entry;
	while( (entry = vfs_readdir(d)) != NULL ) {
		printf( "%s%c    ", entry->name, (entry->type == VFS_INODE_TYPE_DIR ? '/' : ' ') );
	}

	printf( "\n" );

	vfs_closedir( d );
	return 0;
}