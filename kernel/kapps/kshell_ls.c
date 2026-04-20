#include <kernel_common.h>
#include <kshell_app.h>
#include <fs.h>
#include <vfs.h>

KSHELL_COMMAND( ls, kshell_app_ls_main )

char empty_string[] = "";

int kshell_app_ls_main( int argc, char *argv[] ) {
	char *path = NULL;

	if( argc == 2 ) {
		path = argv[1];
	} else {
		path = empty_string;
	}

	char type_dir[] = "DIR ";
	char type_file[] = "FILE";
	char type_unknown[] = "????";

	if( strcmp( path, "" ) == 0 ) {
		strcpy( path, "/" );
	}

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