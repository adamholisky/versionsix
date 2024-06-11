#include <kernel_common.h>
#include <elf.h>

#undef DEBUG_ELF_FILE_CONST
ELF_File::ELF_File( uint64_t *file_start ) {
    file_base = file_start;

    elf_header = (Elf64_Ehdr *)file_base;
    section_headers = (Elf64_Shdr *)((uint64_t)file_base + elf_header->e_shoff);

    Elf64_Shdr* string_table_header = get_section_header( SHT_STRTAB );
    string_table = (char *)((uint64_t)file_base + string_table_header->sh_offset);
    
    Elf64_Shdr* symbol_table_header = get_section_header( SHT_SYMTAB );
    symbol_table = (Elf64_Sym *)((uint64_t)file_base + symbol_table_header->sh_offset);

    num_symbols = symbol_table_header->sh_size/sizeof(Elf64_Sym);

    #ifdef DEBUG_ELF_FILE_CONST
    debugf( "file_base: %016llx\n", file_base );
    debugf( "elf ident: 0x%02X %c %c %c\n", elf_header->e_ident[0], elf_header->e_ident[1], elf_header->e_ident[2], elf_header->e_ident[3] );
    debugf( "Section Header Offset: 0x%llx\n", elf_header->e_shoff );
    debugf( "section headers: %016llx\n", section_headers );

    kdebug_peek_at_n( (uint64_t)string_table, 20 );

    debugf( "Symbol table num entries: %d\n", num_symbols );
    #endif
}

Elf64_Shdr* ELF_File::get_section_header( char *name ) {
    // Don't run if we haven't loaded our string table yet
    if( string_table == NULL ) {
        return NULL;
    }

    return NULL;
}

#undef DEBUG_ELF_GET_SECT_HEADER_TYPE
Elf64_Shdr* ELF_File::get_section_header( int type ) {
    Elf64_Shdr* found_header = NULL;
    
    for( int i = 0; i < elf_header->e_shnum; i++ ) {
        Elf64_Shdr* section = (Elf64_Shdr*)((uint64_t)section_headers + elf_header->e_shentsize*i);

        #ifdef DEBUG_ELF_GET_SECT_HEADER_TYPE
        debugf( "section %d type: %d\n", i, section->sh_type );
        #endif

        if( section->sh_type == type ) {
            i = elf_header->e_shnum + 1;
            found_header = section;
        }
    }

    return found_header;
}

Elf64_Sym* ELF_File::get_symtab( void ) {
    return symbol_table;
}

char* ELF_File::get_strtab( void ) {
    return string_table;
}

char* ELF_File::get_str_at_offset( uint64_t offset ) {
    return (char *)((uint64_t)string_table + offset);
}