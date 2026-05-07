#include <kernel_common.h>
#include <fs.h>
#include <asvfs.h>
#include <asvfs_vios.h>
#include <afs.h>
#include <rfs.h>
#include <vfs.h>
#include <lib/list.h>

char name_stdin[] = "stdin";
char name_stdout[] = "stdout";
char name_stderr[] = "srderr";

avs_list *fd_list;

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

	fd_list = avs_list_init();
	
}

#define KDEBUG_FS_INIT2
void fs_initalize_part2( void ) {

}