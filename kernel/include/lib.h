#if !defined(KLIB_H_INCLUDED)
#define KLIB_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include "elf2.h"

typedef struct _lib_symbol {
	char name[50];
	void * addr;
} lib_symbol;

typedef struct _lib_shared_page {
	void *virt;
	void *phys;
} lib_shared_page;

typedef struct _lib_memory_section {
	bool read;
	bool write;
	bool execute;
	int num_pages;
	int elf_section;
	lib_shared_page *pages;
} lib_memory_section;

typedef struct _lib_shared {
	char name[50];
	uint64_t version;
	
	elf2_file *f_elf;

	uint64_t size_in_memory;
	uint64_t num_mem_sections;
	lib_memory_section *mem_sections;

	uint64_t num_symbols;
	lib_symbol *lib_symbols;

	int total_pages;
	void *lib_base;
	
} lib_shared;

int lib_register( char *pathname );
void lib_initalize( void );
int lib_load_symbols( lib_shared *lib, void *lib_data );
int lib_load_program_headers( lib_shared *lib, void *lib_data );
int lib_load_relocation_tables( lib_shared *lib, void *lib_data );
void *lib_dynamic_linker( char *name );

#ifdef __cplusplus
}
#endif

#endif