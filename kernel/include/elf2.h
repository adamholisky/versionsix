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
	uint64_t 	phys_offset;
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

/* x86-64 relocation types */
#define R_X86_64_NONE		0	/* No reloc */
#define R_X86_64_64		1	/* Direct 64 bit  */
#define R_X86_64_PC32		2	/* PC relative 32 bit signed */
#define R_X86_64_GOT32		3	/* 32 bit GOT entry */
#define R_X86_64_PLT32		4	/* 32 bit PLT address */
#define R_X86_64_COPY		5	/* Copy symbol at runtime */
#define R_X86_64_GLOB_DAT	6	/* Create GOT entry */
#define R_X86_64_JUMP_SLOT	7	/* Create PLT entry */
#define R_X86_64_RELATIVE	8	/* Adjust by program base */
#define R_X86_64_GOTPCREL	9	/* 32 bit signed pc relative offset to GOT */
#define R_X86_64_GOTPCRELX	41
#define R_X86_64_REX_GOTPCRELX	42
#define R_X86_64_32		10	/* Direct 32 bit zero extended */
#define R_X86_64_32S		11	/* Direct 32 bit sign extended */
#define R_X86_64_16		12	/* Direct 16 bit zero extended */
#define R_X86_64_PC16		13	/* 16 bit sign extended pc relative */
#define R_X86_64_8		14	/* Direct 8 bit sign extended  */
#define R_X86_64_PC8		15	/* 8 bit sign extended pc relative */
#define R_X86_64_PC64		24	/* Place relative 64-bit signed */


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