#ifndef VIOS_PROGRAM_INCLUDED
#define VIOS_PROGRAM_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <interrupt.h>
#include <elf.h>

#define PROGRAM_TYPE_LIB 0
#define PROGRAM_TYPE_EXEC 1

typedef struct _program_pages {
	uint64_t phys;
	uint64_t virt;
	uint64_t kern_virt;
} program_pages;

typedef struct _program {
	uint16_t id;
	uint16_t task_id;

	uint8_t type;
	char path[255];	

	elf_file *elf;

	uint32_t num_text_pages;
	uint64_t text_pages_virt_start;
	program_pages *text_pages;

	uint32_t num_data_pages;
	uint64_t data_pages_virt_start;
	program_pages *data_pages;

	struct _program *prev;
	struct _program *next;
} program;

void program_initalize( void );
program *program_load( char *path );
int program_load_data( program *p, void *data, size_t size );
int program_load_elf_module( program *p, void *data, size_t size );
int program_load_elf_library( program *p, void *data, size_t size );
int program_load_elf_binary( program *p, void *data, size_t size );


#ifdef __cplusplus
}
#endif
#endif