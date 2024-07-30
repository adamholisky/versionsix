#include <kernel_common.h>
#include <kshell_app.h>
#include <fs.h>

KSHELL_COMMAND( cat, kshell_app_cat_main )

int kshell_app_cat_main( int argc, char *argv[] ) {
	char *path = NULL;

	if( argc == 2 ) {
		path = argv[1];

		vfs_stat_data stats;

		int stat_error = vfs_stat( vfs_lookup_inode(path), &stats );
		if( stat_error != VFS_ERROR_NONE ) {
			printf( "Error: %d\n", stat_error );
			return 1;
		}

		char *data = vfs_malloc( stats.size );
		int read_err = vfs_read( vfs_lookup_inode(path), data, stats.size, 0 );
		if( read_err < VFS_ERROR_NONE ) {
			printf( "Error when reading: %d\n", read_err );
			return 1;
		}

		data[ stats.size ] = 0;
		printf( "%s", data );
		printf( "\n" );
	} else {
		/* printf( "Missing filename.\n" );
        return 1; */
		char picard[] = "/home/adam/picardhistory.txt";
		path = picard;

		vfs_stat_data stats;

		int stat_error = vfs_stat( vfs_lookup_inode(path), &stats );
		if( stat_error != VFS_ERROR_NONE ) {
			printf( "Error: %d\n", stat_error );
			return 1;
		}

		char *data = vfs_malloc( stats.size );
		int read_err = vfs_read( vfs_lookup_inode(path), data, stats.size, 0 );
		if( read_err < VFS_ERROR_NONE ) {
			printf( "Error when reading: %d\n", read_err );
			return 1;
		}

		data[ 5000 ] = 0;
		printf( "%s", data );
		printf( "\n" );
	}

	

	return 0;
}