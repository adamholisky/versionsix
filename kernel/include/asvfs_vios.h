#if !defined(ASVFS_VIOS_INCLUDED)
#define ASVFS_VIOS_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

int asvfs_vios_init( int drive_id );
int asvfs_vios_get_drive_id( void );

int asvfs_get_dir_list_glue( char *pathname, vfs_directory_list *dlist );

#ifdef __cplusplus
}
#endif

#endif