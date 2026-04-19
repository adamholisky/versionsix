#include <kernel_common.h>
#include <fs.h>
#include <asvfs.h>
#include <asvfs_vios.h>
#include <afs.h>
#include <rfs.h>
#include <vfs.h>

char name_stdin[] = "stdin";
char name_stdout[] = "stdout";
char name_stderr[] = "srderr";

/**
 * @brief Initalize the filesystem
 * 
 */
#define KDEBUG_FS_INIT1
void fs_initalize_part1( void ) {
	int vfs_init_err = vfs_initalize();
	if( vfs_init_err != VFS_ERROR_NONE ) {
		klog( LOG_ERROR, "VFS Initalization failed: %d", vfs_init_err );
		
		return;
	} else {
		klog( LOG_INFO, "VFS initalized." );
	}

	int asvfs_init_err = asvfs_vios_init( FS_DRIVE_1 );
	if( vfs_init_err != VFS_ERROR_NONE ) {
		klog( LOG_ERROR, "ASVFS initalization failed: %d\n", vfs_init_err );
	} else {
		klog( LOG_INFO, "ASVFS initalized." );
	}

	int mount_err = vfs_mount( "ASVFS", "/", 0 );

	int rfs_init_err = rfs_initalize();
	if( rfs_init_err != VFS_ERROR_NONE ) {
		klog( LOG_ERROR, "RFS Initalization failed: %d\n", rfs_init_err );
		
		return;
	} else {
		klog( LOG_INFO, "RFS initalized." );
	}

	mount_err = vfs_mount( "RFS", "/dev", 0 );


/* 	int afs_init_err = afs_initalize();
	if( afs_init_err != VFS_ERROR_NONE ) {
		debugf( "AFS Initalization failed: %d\n", afs_init_err );
		
		return;
	} else {
		debugf( "AFS initalized.\n" );
	}

	int rfs_init_err = rfs_initalize();
	if( rfs_init_err != VFS_ERROR_NONE ) {
		debugf( "RFS Initalization failed: %d\n", rfs_init_err );
		
		return;
	} else {
		debugf( "RFS initalized.\n" );
	}

	 int afs_mount_err = vfs_mount( FS_TYPE_AFS, NULL, "/" );
	if( afs_mount_err != 0 ) {
		debugf( "Could not mount afs drive.\n" );

		return;
	} else {
		debugf( "Mounted afs on /.\n" );
	}

	int asvfs_mount_err = vfs_mount( FS_TYPE_ASVFS, NULL, "/" );
	if( asvfs_mount_err != 0 ) {
		klog( LOG_ERROR, "Could not mount asvfs drive.\n" );

		return;
	} else {
		klog( LOG_INFO, "Mounted asvfs on /.\n" );
	}
	

	// Directory for devices
	vfs_mkdir( 1, "/", "dev" );
	vfs_mkdir( 1, "/", "proc" );

	// Mount RFS
	int rfs_mount_err = vfs_mount( FS_TYPE_RFS, NULL, "/proc" );
	if( rfs_mount_err != 0 ) {
		debugf( "Could not mount /proc fs.\n" );

		return;
	} else {
		vfs_debugf( "Mounted rfs on /proc.\n" );
	}	
	
	int rfs_mount_err2 = vfs_mount( FS_TYPE_RFS, NULL, "/dev" );
	if( rfs_mount_err2 != 0 ) {
		debugf( "Could not mount /dev fs.\n" );

		return;
	} else {
		vfs_debugf( "Mounted rfs on /dev.\n" );
	}	 */
}

#define KDEBUG_FS_INIT2
void fs_initalize_part2( void ) {

}