#ifndef VIOS_LIMINE_BOOTSTRAP_INCLUDED
#define VIOS_LIMINE_BOOTSTRAP_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void load_limine_info( void );
char *limine_mem_map_type_to_text( uint8_t type );

#ifdef __cplusplus
}
#endif
#endif