#include <kernel_common.h>
#include <elf.h>
#include <kmemory.h>

#define DEBUG_ELF_FILE_CONST
void elf_file_initalize( elf_file *elf, uint64_t *file_start ) {
	elf->file_base = file_start;

	elf->elf_header = (Elf64_Ehdr *)elf->file_base;
	elf->section_headers = (Elf64_Shdr *)((uint64_t)elf->file_base + elf->elf_header->e_shoff);

	//Elf64_Shdr* string_table_header = elf_get_section_header( elf, SHT_STRTAB );
	Elf64_Shdr *string_table_header = elf_get_section_header_by_index( elf, elf->elf_header->e_shstrndx );
	elf->string_table = (char *)((uint64_t)elf->file_base + string_table_header->sh_offset);
	
	Elf64_Shdr* symbol_table_header = elf_get_section_header_by_type( elf, SHT_SYMTAB );
	elf->symbol_table = (Elf64_Sym *)((uint64_t)elf->file_base + symbol_table_header->sh_offset);

	elf->num_symbols = symbol_table_header->sh_size/sizeof(Elf64_Sym);
	elf->num_program_headers = elf->elf_header->e_phnum;
	elf->program_headers = (Elf64_Phdr *)((uint8_t *)elf->file_base + elf->elf_header->e_phoff);


	#ifdef DEBUG_ELF_FILE_CONST
	debugf( "file_base: %016llx\n", elf->file_base );
	debugf( "elf ident: 0x%02X %c %c %c\n", elf->elf_header->e_ident[0], elf->elf_header->e_ident[1], elf->elf_header->e_ident[2], elf->elf_header->e_ident[3] );
	debugf( "Section Header Offset: 0x%llx\n", elf->elf_header->e_shoff );
	debugf( "section headers: %016llx\n", elf->section_headers );
	debugf( "Symbol table: %016llX\n", elf->symbol_table );
	debugf( "String table: %016llx\n", elf->string_table );
	debugf( "Ph number: %d\n", elf->num_program_headers );
	debugf( "Ph Offset: 0x%llx\n", elf->elf_header->e_phoff );
	debugf( "Ph Addr: 0x%llx\n", elf->program_headers );

	kdebug_peek_at_n( (uint64_t)elf->string_table, 15 );

	debugf( "Symbol table num entries: %d\n", elf->num_symbols );
	#endif
}

/**
 * @brief 
 * 
 * @param elf 
 * @param name 
 * @return Elf64_Shdr* 
 */
Elf64_Shdr* elf_get_section_header_by_name( elf_file *elf, char *name ) {
	if( elf == NULL ) {
		return NULL;
	}

	// Don't run if we haven't loaded our string table yet
	if( elf->string_table == NULL ) {
		return NULL;
	}

	Elf64_Shdr* found_header = NULL;
	
	for( int i = 0; i < elf->elf_header->e_shnum; i++ ) {
		Elf64_Shdr* section = (Elf64_Shdr*)((uint8_t *)elf->section_headers + (elf->elf_header->e_shentsize*i));

		char *section_name = elf_get_str_at_offset( elf, section->sh_name );

		debugf( "section %d type: %d  offset: 0x%X name: %s\n", i,  section->sh_type, section->sh_name, section_name );

		if( strcmp( name, section_name ) == 0 ) {
			i = elf->elf_header->e_shnum + 1;
			found_header = section;
		}
	}

	return found_header;
}

/**
 * @brief 
 * 
 */
#undef DEBUG_ELF_GET_SECT_HEADER_TYPE
Elf64_Shdr* elf_get_section_header_by_type( elf_file *elf, int type ) {
	Elf64_Shdr* found_header = NULL;
	
	for( int i = 0; i < elf->elf_header->e_shnum; i++ ) {
		Elf64_Shdr* section = (Elf64_Shdr*)((uint64_t)elf->section_headers + elf->elf_header->e_shentsize*i);

		#ifdef DEBUG_ELF_GET_SECT_HEADER_TYPE
		debugf( "section %d type: %d\n", i, section->sh_type );
		#endif

		if( section->sh_type == type ) {
			i = elf->elf_header->e_shnum + 1;
			found_header = section;
		}
	}

	return found_header;
}

Elf64_Shdr *elf_get_section_header_by_index( elf_file *elf, uint8_t index ) {
	return (Elf64_Shdr*)((uint64_t)elf->section_headers + elf->elf_header->e_shentsize*(index - 1));
}

Elf64_Sym* elf_get_symtab( elf_file *elf ) {
	return elf->symbol_table;
}

char* elf_get_strtab( elf_file *elf ) {
	return elf->string_table;
}

char* elf_get_str_at_offset( elf_file *elf, uint64_t offset ) {
	return (char *)((uint64_t)elf->string_table + offset);
}

Elf64_Phdr *get_program_header_by_index( elf_file *elf, uint8_t index ) {
	return (Elf64_Phdr *)((uint8_t *)elf->file_base + elf->elf_header->e_phoff + (index * elf->elf_header->e_phentsize)); 
}

/**
 * @brief 
 * 
 * @param elf 
 * @return int 
 */
#undef KDEBUG_ELF_LOAD_SYMBOLS
int elf_load_symbols( elf_file *elf ) {
	Elf64_Sym* symbol_table = elf_get_symtab( elf );

	elf->symbols = kmalloc( sizeof(symbol_collection) );

	#ifdef KDEBUG_ELF_LOAD_SYMBOLS
	debugf( "Symbol list at load: \n" );
	#endif
	for( int i = 0; i < elf->num_symbols; i++ ) {
		Elf64_Sym* sym = (Elf64_Sym*)((uint64_t)symbol_table + sizeof(Elf64_Sym)*i); 
		
		#ifdef KDEBUG_ELF_LOAD_SYMBOLS
		debugf( "%d: 0x%llx %d   %s   %s   %x %s\n",
			i, 
			sym->st_value, 
			sym->st_size, 
			elf_type_to_str(ELF32_ST_TYPE(sym->st_info)), 
			elf_bind_to_str(ELF32_ST_BIND(sym->st_info)),
			sym->st_name,
			elf_get_str_at_offset( elf, sym->st_name )
		);
		#endif
		
		symbols_add( elf->symbols, 
					elf_get_str_at_offset( elf, sym->st_name), 
					sym->st_value, 
					sym->st_size 
					);
	}
}

char *elf_type_to_str( uint8_t type ) {
	switch( type ) {
		case STT_NOTYPE:
			return "NOTYPE";
		case STT_OBJECT:
			return "OBJECT";
		case STT_FUNC:
			return "FUNC";
		case STT_SECTION:
			return "SECTION";
		case STT_FILE:
			return "FILE";
		case STT_LOPROC:
			return "LOPROC";
		case STT_HIPROC:
			return "HIPROC";
		default:
			return "?????";
	}
}

char *elf_bind_to_str( uint8_t bind ) {
	switch( bind ) {
		case STB_LOCAL:
			return "LOCAL";
		case STB_GLOBAL:
			return "GLOBAL";
		case STB_WEAK:
			return "WEAK";
		case STB_LOPROC:
			return "LOPROC";
		case STB_HIPROC:
			return "HIPROC";
	}
}