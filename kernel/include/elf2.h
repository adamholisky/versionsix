#if !defined(ELF2_H_INCLUDED)
#define ELF2_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <kernel_common.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdbool.h>

#include <elf.h>

typedef struct {
	char * 		name;
	uint64_t	addr;
	uint64_t	size;
	uint32_t	sh_index;
	uint32_t	type;
	uint8_t		binding;
} elf2_symbol;

typedef struct {
	uint32_t	type;
	uint64_t	phys_addr;
	uint64_t	virt_addr;
	uint64_t	phys_size;
	uint64_t	virt_size;
	bool		read;
	bool		write;
	bool		execute;
} elf2_program_header;

typedef struct {
	// Basics
	void *file_base;
	Elf64_Ehdr *header;

	// Section headers
	Elf64_Shdr *section_headers;
	char *str_tbl_section_names;

	// Program headers
	bool has_program_headers;
	int program_headers_num_ents;
	int program_headers_num_loads;
	elf2_program_header *program_headers;

	// Dynamic entries
	bool has_dynsym;
	int dynsym_num_ents;
	char *dynsym_str_tbl;
	elf2_symbol *dynsym_ents;


	Elf64_Sym* symbol_table;
	uint64_t num_symbols;
} elf2_file;

#define ELF_ST_BIND(x)		((x) >> 4)
#define ELF_ST_TYPE(x)		((x) & 0xf)
#define ELF64_ST_BIND(x)	ELF_ST_BIND(x)
#define ELF64_ST_TYPE(x)	ELF_ST_TYPE(x)


int elf2_new_initalize_file( elf2_file *elf, void *data );
Elf64_Shdr* elf2_new_get_section_header_by_name( elf2_file *elf, char* name );
Elf64_Shdr* elf2_new_get_section_header_by_index( elf2_file *elf, uint8_t index );
Elf64_Shdr* elf2_new_get_section_header_by_type( elf2_file *elf, int type );
Elf64_Sym* elf2_new_get_symbol_entry( elf2_file *f_elf, Elf64_Off s_offset, uint8_t index );
Elf64_Phdr *elf2_new_get_program_header_by_index( elf2_file *f_elf, uint8_t index );


#ifdef __cplusplus
}
#endif

#endif