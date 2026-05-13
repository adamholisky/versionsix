#if !defined(ELF2_H_INCLUDED)
#define ELF2_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "elf.h"

#define KERROR_UNKNOWN -1
#define KERROR_NONE 0
#define VFS_ERROR_NONE 0
#define kstrcmp strcmp
#define KASSERT_NOT_NULL_R(x) if(x == NULL) { printf( "Assert not null failed on " #x ); return 0; }

typedef struct {
	void *file_base;

	Elf64_Ehdr *header;
	Elf64_Shdr *section_headers;

	char *str_tbl;
	char *str_tbl_section_names;

	Elf64_Sym* symbol_table;
	uint64_t num_symbols;
} elf_new_file;

int elf_new_initalize_file( elf_new_file *elf, void *data );
Elf64_Shdr* elf_new_get_section_header_by_name( elf_new_file *elf, char* name );
Elf64_Shdr* elf_new_get_section_header_by_index( elf_new_file *elf, uint8_t index );
Elf64_Shdr* elf_new_get_section_header_by_type( elf_new_file *elf, int type );

#ifdef __cplusplus
}
#endif

#endif