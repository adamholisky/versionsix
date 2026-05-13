#include <stdio.h>
#include "elfold.h"
#include "elf.h"

/**
 * @brief Initalize the elf_file structure with the file's data
 * 
 * @param f_elf Pointer to allocated elf_file
 * @param data Pointer to the file's data 
 * @return int 0 on no error, otherwise kerror_no
 */
int elf_new_initalize_file( elf_new_file *f_elf, void *data ) {
	KASSERT_NOT_NULL_R(f_elf);
	KASSERT_NOT_NULL_R(data);

	f_elf->file_base = data;
	f_elf->header = (Elf64_Ehdr *)f_elf->file_base;
	f_elf->section_headers = (Elf64_Shdr *)(f_elf->file_base + f_elf->header->e_shoff);

	// Get the string table	
	Elf64_Shdr *sh_str_tbl = elf_new_get_section_header_by_index( f_elf, f_elf->header->e_shstrndx + 1 );
	f_elf->str_tbl = (f_elf->file_base + sh_str_tbl->sh_offset );

	// Get the section name table
	Elf64_Shdr *sh_section_name_tbl = elf_new_get_section_header_by_index( f_elf, f_elf->header->e_shstrndx + 1 );
	f_elf->str_tbl_section_names = (f_elf->file_base + sh_section_name_tbl->sh_offset);

	return KERROR_NONE;
}

Elf64_Shdr* elf_new_get_section_header_by_name( elf_new_file *f_elf, char* name ) {
	if( f_elf == NULL ) {
		return NULL;
	}

	// Don't run if we haven't loaded our string table yet
	if( f_elf->str_tbl == NULL ) {
		return NULL;
	}

	Elf64_Shdr *found_header = NULL;
	
	for( int i = 0; i < f_elf->header->e_shnum; i++ ) {
		Elf64_Shdr *section = (Elf64_Shdr*)((uint8_t *)f_elf->section_headers + (f_elf->header->e_shentsize*i));

		char *section_name = (char *)((uint8_t *)f_elf->str_tbl_section_names + section->sh_name);

		printf( "section %d type: %d  offset: 0x%X name: %s\n", i,  section->sh_type, section->sh_name, section_name );

		if( kstrcmp( name, section_name ) == 0 ) {
			found_header = section;
			break;
		}
	}

	return found_header;
}

Elf64_Shdr *elf_new_get_section_header_by_index( elf_new_file *f_elf, uint8_t index ) {
	return (Elf64_Shdr*)((uint8_t)f_elf->section_headers + f_elf->header->e_shentsize*(index - 1));
}

Elf64_Shdr* elf_new_get_section_header_by_type( elf_new_file *f_elf, int type ) {

}