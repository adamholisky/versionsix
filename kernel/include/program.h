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

void program_initalize( void );
int program_load( char *path );
int program_load_data( program *p, void *data, size_t size );
int program_load_elf_module( program *p, void *data, size_t size );
int program_load_elf_library( program *p, void *data, size_t size );
int program_load_elf_binary( program *p, void *data, size_t size );


#ifdef __cplusplus
}
#endif
#endif