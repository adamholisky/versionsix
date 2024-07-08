#ifndef VIOS_ELF_INCLUDED
#define VIOS_ELF_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* elf.h
* 
* Most structs are from the ELF/Linux Kernel docs
*/

#define Elf32_Half uint16_t
#define Elf32_Word uint32_t
#define Elf32_Sword int32_t
#define Elf32_Addr uint32_t
#define Elf32_Off uint32_t

#define Elf64_Addr uint64_t
#define Elf64_Half uint16_t
#define Elf64_SHalf int16_t
#define Elf64_Off uint64_t
#define Elf64_Sword int32_t
#define Elf64_Word uint32_t
#define Elf64_Xword uint32_t
#define Elf64_Sxword int64_t

#define EI_NIDENT 16

#define ET_NONE 0 // No file type
#define ET_REL 1 // Relocatable file
#define ET_EXEC 2 // Executable file
#define ET_DYN 3 // Shared object file
#define ET_CORE 4 //Core file
#define ET_LOPROC 0xFF00 // Processor-specific
#define ET_HIPROC 0xFFFF // Processor-specific

#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11

#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MASKPROC 0xF0000000

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2

#define ELF32_R_SYM(i) ((i)>>8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t))

#define STT_FUNC 2
#define ELF32_ST_BIND(INFO)	((INFO) >> 4)
#define ELF32_ST_TYPE(INFO)	((INFO) & 0x0F)

typedef struct {
	unsigned char	e_ident[EI_NIDENT];
	Elf64_Half e_type;
	Elf64_Half e_machine;
	Elf64_Word e_version;
	Elf64_Addr e_entry;
	Elf64_Off e_phoff;
	Elf64_Off e_shoff;
	Elf64_Word e_flags;
	Elf64_Half e_ehsize;
	Elf64_Half e_phentsize;
	Elf64_Half e_phnum;
	Elf64_Half e_shentsize;
	Elf64_Half e_shnum;
	Elf64_Half e_shstrndx;
} Elf64_Ehdr;


typedef struct {
	Elf64_Word p_type;
	Elf64_Word p_flags;
	Elf64_Off p_offset;	
	Elf64_Addr p_vaddr;	
	Elf64_Addr p_paddr;	
	Elf64_Xword p_filesz;	
	Elf64_Xword p_memsz;	
	Elf64_Xword p_align;
} Elf64_Phdr;


typedef struct {
	Elf64_Word sh_name;
	Elf64_Word sh_type;
	Elf64_Xword sh_flags;	
	Elf64_Addr sh_addr;	
	Elf64_Off sh_offset;
	Elf64_Xword sh_size;	
	Elf64_Word sh_link;	
	Elf64_Word sh_info;		
	Elf64_Xword sh_addralign;	
	Elf64_Xword sh_entsize;
} Elf64_Shdr;

typedef struct {
	Elf64_Word st_name;		
	unsigned char	st_info;
	unsigned char	st_other;	
	Elf64_Half st_shndx;
	Elf64_Addr st_value;
	Elf64_Xword st_size;
} Elf64_Sym;

typedef struct {
	Elf64_Addr r_offset;
	Elf64_Xword r_info;
} Elf64_Rel;

typedef struct {
	Elf64_Addr r_offset;
	Elf64_Xword r_info;	
	Elf64_Sxword r_addend;
} Elf64_Rela;

typedef struct {
	uint64_t* file_base;

	Elf64_Ehdr* elf_header;
	Elf64_Shdr* section_headers;
	char* string_table;
	Elf64_Sym* symbol_table;
	uint64_t num_symbols;
} elf_file;

void elf_file_initalize( elf_file *elf, uint64_t *file_start );
Elf64_Shdr* elf_get_section_header_by_name( elf_file *elf, char* name );
Elf64_Shdr* elf_get_section_header( elf_file *elf, int type );
Elf64_Sym* elf_get_symtab( elf_file *elf );
char* elf_get_strtab( elf_file *elf );
char* elf_get_str_at_offset( elf_file *elf, uint64_t offset );

#ifdef __cplusplus
}
#endif
#endif