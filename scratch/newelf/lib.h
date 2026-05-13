#if !defined(KLIB_H_INCLUDED)
#define KLIB_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>

void lib_initalize( void );
int lib_register( char* pathname );

#ifdef __cplusplus
}
#endif

#endif