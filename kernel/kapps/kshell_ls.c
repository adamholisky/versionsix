#include <kernel_common.h>
#include <kshell_app.h>
#include <fs.h>

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

	vfs_directory_list *dir_list = vfs_malloc( sizeof(vfs_directory_list) );
	vfs_get_directory_list( vfs_lookup_inode(path), dir_list );

	if( dir_list == NULL ) {
		printf( "Directory not found.\n" );
		return 1;
	}

	for( int i = 0; i < dir_list->count; i++ ) {
		vfs_inode *n = vfs_lookup_inode_ptr_by_id( dir_list->entry[i].id );
		char dir_char = (n->type == VFS_INODE_TYPE_DIR ? '/' : ' ');

		printf( "%s%c    ", dir_list->entry[i].name, dir_char );
		// old way, ressurect this at some point
		/* char *type = NULL;

		vfs_inode *n = vfs_lookup_inode_ptr_by_id( dir_list->entry[i].id );
		switch( n->type ) {
			case VFS_INODE_TYPE_DIR:
				type = type_dir;
				break;
			case VFS_INODE_TYPE_FILE:
				type = type_file;
				break;
			default:
				type = type_unknown;
		}

		printf( "    %03ld %s %s\n", dir_list->entry[i].id, type, dir_list->entry[i].name ); */
	}

	printf( "\n" );
	return 0;
}