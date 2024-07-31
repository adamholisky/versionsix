#if !defined(FS_INCLUDED)
#define FS_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <unistd.h>
#include <vfs.h>

void fs_initalize_part1( void );
void fs_initalize_part2( void );

#ifdef __cplusplus
}
#endif

#endif