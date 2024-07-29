#include <kernel_common.h>
#include <fs.h>
#include <afs.h>
#include <rfs.h>

char name_stdin[] = "stdin";
char name_stdout[] = "stdout";
char name_stderr[] = "srderr";

/**
 * @brief Initalize the filesystem
 * 
 */
#define KDEBUG_FS_INIT
void fs_initalize( void ) {
	int vfs_init_err = vfs_initalize();
	if( vfs_init_err != VFS_ERROR_NONE ) {
		debugf( "VFS Initalization failed: %d\n", vfs_init_err );
		
		return;
	}

	int afs_init_err = afs_initalize();
	if( afs_init_err != VFS_ERROR_NONE ) {
		debugf( "AFS Initalization failed: %d\n", afs_init_err );
		
		return;
	}

	int rfs_init_err = rfs_initalize();
	if( rfs_init_err != VFS_ERROR_NONE ) {
		debugf( "RFS Initalization failed: %d\n", rfs_init_err );
		
		return;
	}

	int afs_mount_err = vfs_mount( FS_TYPE_AFS, NULL, "/" );
	if( afs_mount_err != 0 ) {
		debugf( "Could not mount afs drive.\n" );

		return;
	}
	debugf( "Mounted afs on /.\n" );

	// Directory for RFS
	vfs_mkdir( 1, "/", "proc" );

	// Mount RFS
	int rfs_mount_err = vfs_mount( FS_TYPE_RFS, NULL, "/proc" );
	if( rfs_mount_err != 0 ) {
		debugf( "Could not mount /proc fs.\n" );

		return;
	}
	vfs_debugf( "Mounted rfs on /proc.\n" );
}