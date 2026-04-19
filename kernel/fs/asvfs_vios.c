#include <kernel_common.h>
#include <fs.h>
#include <vfs.h>
#include <asvfs.h>
#include <asvfs_vios.h>

asvfs_drive_ops asvfs_ops;
vfs_operations asvfs_vfs_ops;
vfs_filesystem *asvfs;

int asvfs_vios_init( int drive_id ) {
	asvfs_ops.read_from_disk = vfs_disk_read;
	asvfs_ops.write_to_disk = vfs_disk_write;
	
	int asvfs_init_err = asvfs_initalize( &asvfs_ops );
	if( asvfs_init_err != ASVFS_ERROR_NONE ) {
		return asvfs_init_err;
	}

	asvfs_vfs_ops.get_dir_list = asvfs_get_dir_list_glue;
	asvfs_vfs_ops.read = asvfs_read;
	asvfs_vfs_ops.write = asvfs_write;
	asvfs_vfs_ops.create = asvfs_create;
	asvfs_vfs_ops.open = asvfs_open;
	asvfs_vfs_ops.getattr = asvfs_getattr;

	asvfs = vfs_register_fs( "ASVFS", &asvfs_vfs_ops );
	if( asvfs == NULL ) {
		klog( LOG_DEBUG, "Register FS error. FS obj is NULL." );
		return -1;
	}

	asvfs->type = FS_TYPE_ASVFS;
	asvfs->drive_id = drive_id;
	
}

int asvfs_vios_get_drive_id( void ) {
	return asvfs->drive_id;
}


int asvfs_get_dir_list_glue( char *pathname, vfs_directory_list *dlist ) {

}