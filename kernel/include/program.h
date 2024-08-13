#ifndef VIOS_PROGRAM_INCLUDED
#define VIOS_PROGRAM_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <interrupt.h>

#define PROGRAM_TYPE_LIB 0
#define PROGRAM_TYPE_EXEC 1

typedef struct _program {
	uint16_t id;

	uint8_t type;
	char path[255];	

	struct _program *prev;
	struct _program *next;
} program;





int program_load( char *path );
int program_load_data( void *data, size_t size );
int program_load_elf_module( void *data, size_t size );
int program_load_elf_library( void *data, size_t size );
int program_load_elf_binary( void *data, size_t size );


#ifdef __cplusplus
}
#endif
#endif