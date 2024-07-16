#if !defined(FS_INCLUDED)
#define FS_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <afs.h>
#include <unistd.h>

void fs_initalize( void );
vv_file_internal * get_fs_internal( void );
uint32_t get_file_size( int _FD );

void primative_ls( char *path );
void primative_pwd( void );
void primative_cat( char *pathname );


#ifdef __cplusplus
}
#endif

#endif