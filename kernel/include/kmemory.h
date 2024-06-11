#ifndef VIOS_MEMORY_INCLUDED
#define VIOS_MEMORY_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

void memory_initalize( void );
void *kmalloc( uint64_t size );
void kfree( uint64_t *p );

#ifdef __cplusplus
}
#endif
#endif