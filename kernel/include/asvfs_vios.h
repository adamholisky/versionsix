#if !defined(ASVFS_VIOS_INCLUDED)
#define ASVFS_VIOS_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

int asvfs_vios_init( int drive_id );
int asvfs_vios_get_drive_id( void );

int asvfs_get_dir_list_glue( char *pathname, vfs_dir *dirp );
int asvfs_close_dir_list_glue( vfs_dir *dirp );



#ifdef __cplusplus
}
#endif

#endif