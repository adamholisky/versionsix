#include <kernel_common.h>
#include <elf.h>

#undef DEBUG_ELF_FILE_CONST
void elf_file_initalize( elf_file *elf, uint64_t *file_start ) {
    elf->file_base = file_start;

    elf->elf_header = (Elf64_Ehdr *)elf->file_base;
    elf->section_headers = (Elf64_Shdr *)((uint64_t)elf->file_base + elf->elf_header->e_shoff);

    Elf64_Shdr* string_table_header = elf_get_section_header( elf, SHT_STRTAB );
    elf->string_table = (char *)((uint64_t)elf->file_base + string_table_header->sh_offset);
    
    Elf64_Shdr* symbol_table_header = elf_get_section_header( elf, SHT_SYMTAB );
    elf->symbol_table = (Elf64_Sym *)((uint64_t)elf->file_base + symbol_table_header->sh_offset);

    elf->num_symbols = symbol_table_header->sh_size/sizeof(Elf64_Sym);

    #ifdef DEBUG_ELF_FILE_CONST
    debugf( "file_base: %016llx\n", file_base );
    debugf( "elf ident: 0x%02X %c %c %c\n", elf_header->e_ident[0], elf_header->e_ident[1], elf_header->e_ident[2], elf_header->e_ident[3] );
    debugf( "Section Header Offset: 0x%llx\n", elf_header->e_shoff );
    debugf( "section headers: %016llx\n", section_headers );

    kdebug_peek_at_n( (uint64_t)string_table, 20 );

    debugf( "Symbol table num entries: %d\n", num_symbols );
    #endif
}

Elf64_Shdr* elf_get_section_header_by_name( elf_file *elf, char *name ) {
    // Don't run if we haven't loaded our string table yet
    if( elf->string_table == NULL ) {
        return NULL;
    }

    return NULL;
}

#undef DEBUG_ELF_GET_SECT_HEADER_TYPE
Elf64_Shdr* elf_get_section_header( elf_file *elf, int type ) {
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

Elf64_Sym* elf_get_symtab( elf_file *elf ) {
    return elf->symbol_table;
}

char* elf_get_strtab( elf_file *elf ) {
    return elf->string_table;
}

char* elf_get_str_at_offset( elf_file *elf, uint64_t offset ) {
    return (char *)((uint64_t)elf->string_table + offset);
}