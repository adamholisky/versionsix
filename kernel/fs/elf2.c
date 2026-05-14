#include <kernel_common.h>
#include <vfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <elf2.h>

/**
 * @brief Initalize the elf_file structure with the file's data
 * 
 * @param f_elf Pointer to allocated elf_file
 * @param data Pointer to the file's data 
 * @return int 0 on no error, otherwise kerror_no
 */
int elf2_new_initalize_file( elf2_file *f_elf, void *data ) {
	KASSERT_NOT_NULL_R(f_elf);
	KASSERT_NOT_NULL_R(data);

	f_elf->file_base = data;
	f_elf->header = (Elf64_Ehdr *)f_elf->file_base;
	f_elf->section_headers = (Elf64_Shdr *)((uint8_t *)f_elf->file_base + f_elf->header->e_shoff);

	printf( "f_elf->file_base: %016llX\n", f_elf->file_base );
	printf( "f_elf->header: %016llX\n", f_elf->header );
	printf( "f_elf->e_shoff: %016llX\n", f_elf->header->e_shoff );
	printf( "f_elf->section_headers: %016llX\n", f_elf->section_headers );

	// Get the section name table
	Elf64_Shdr *sh_section_name_tbl = elf2_new_get_section_header_by_index( f_elf, f_elf->header->e_shstrndx + 1 );
	f_elf->str_tbl_section_names = (f_elf->file_base + sh_section_name_tbl->sh_offset);

	// Load the program headers, leave it up to the user of this data to allocate, compute, etc...
	f_elf->program_headers_num_ents = f_elf->header->e_phnum;
	if( f_elf->program_headers_num_ents > 0 ) {
		f_elf->has_program_headers = true;

		f_elf->program_headers = malloc( sizeof(elf2_program_header) * f_elf->program_headers_num_ents );
		if( f_elf->program_headers != NULL ) {
			for( int i = 0; i < f_elf->program_headers_num_ents; i++ ) {
				Elf64_Phdr *phdr = elf2_new_get_program_header_by_index( f_elf, i );

				f_elf->program_headers[i].phys_addr = phdr->p_paddr;
				f_elf->program_headers[i].phys_offset = phdr->p_offset;
				f_elf->program_headers[i].virt_addr = phdr->p_vaddr;
				f_elf->program_headers[i].type = phdr->p_type;
				f_elf->program_headers[i].phys_size = phdr->p_filesz;
				f_elf->program_headers[i].virt_size = phdr->p_memsz;
				f_elf->program_headers[i].read = phdr->p_flags & PF_R;
				f_elf->program_headers[i].write = phdr->p_flags & PF_W;
				f_elf->program_headers[i].execute = phdr->p_flags & PF_X;

				if( phdr->p_type == PT_LOAD ) { f_elf->program_headers_num_loads++; }

				printf( "Added PH: %d \t %d \t phys 0x%016llX \t offset 0x%0X \t %X \t virt 0x%016llX \t %X RWX: %d %d %d\n", i, f_elf->program_headers[i].type, f_elf->program_headers[i].phys_addr, f_elf->program_headers[i].phys_offset, f_elf->program_headers[i].phys_size, f_elf->program_headers[i].virt_addr, f_elf->program_headers[i].virt_size, f_elf->program_headers[i].read, f_elf->program_headers[i].write, f_elf->program_headers[i].execute );
			}
		}
	}

	// Have a dynamic section? Load the data now.
	Elf64_Shdr *sh_dyn = elf2_new_get_section_header_by_name( f_elf, ".dynsym" );
	if( sh_dyn != NULL ) {
		f_elf->has_dynsym = true;
		f_elf->dynsym_num_ents = sh_dyn->sh_size / sh_dyn->sh_entsize;

		// We need the dynstr section as well
		Elf64_Shdr *sh_dyn_str = elf2_new_get_section_header_by_name( f_elf, ".dynstr" );
		if( sh_dyn_str != NULL ) {
			f_elf->dynsym_str_tbl = f_elf->file_base + sh_dyn_str->sh_offset;

			// Populate our dynamic symbol array
			f_elf->dynsym_ents = malloc( sizeof(elf2_symbol) * f_elf->dynsym_num_ents);

			if( f_elf->dynsym_ents != NULL ) {
				for( int i = 0; i < f_elf->dynsym_num_ents; i++ ) {
					Elf64_Sym *s_dyn = elf2_new_get_symbol_entry( f_elf, sh_dyn->sh_offset, i );

					f_elf->dynsym_ents[i].name = (char *)((uint8_t *)f_elf->dynsym_str_tbl + s_dyn->st_name);
					f_elf->dynsym_ents[i].addr = s_dyn->st_value;
					f_elf->dynsym_ents[i].size = s_dyn->st_size;
					f_elf->dynsym_ents[i].sh_index = s_dyn->st_shndx;
					f_elf->dynsym_ents[i].type = ELF64_ST_TYPE( s_dyn->st_info );
					f_elf->dynsym_ents[i].binding = ELF64_ST_BIND( s_dyn->st_info );

					if( f_elf->dynsym_ents[i].sh_index != 0 ) { f_elf->num_symbols++; }

					//printf( "Added: 0x%X \t for 0x%X \t idx %d \t type %d \t %s \n", f_elf->dynsym_ents[i].addr, f_elf->dynsym_ents[i].size, f_elf->dynsym_ents[i].sh_index, f_elf->dynsym_ents[i].type, f_elf->dynsym_ents[i].name );
				}
			} else {
				f_elf->has_dynsym = false;
				printf( "dynsym_ents is null\n" );
			}
		} else {
			f_elf->has_dynsym = false;
			printf( "sh_dyn_str is null\n" );
		}
	} else {
		printf( "sh_dyn is null\n" );
	}

	return KERR_NONE;
}

Elf64_Shdr* elf2_new_get_section_header_by_name( elf2_file *f_elf, char* name ) {
	if( f_elf == NULL ) {
		return NULL;
	}

	// Don't run if we haven't loaded our string table yet
	if( f_elf->str_tbl_section_names == NULL ) {
		return NULL;
	}

	Elf64_Shdr *found_header = NULL;
	
	for( int i = 0; i < f_elf->header->e_shnum; i++ ) {
		Elf64_Shdr *section = (Elf64_Shdr*)((uint8_t *)f_elf->section_headers + (f_elf->header->e_shentsize*i));

		char *section_name = (char *)((uint8_t *)f_elf->str_tbl_section_names + section->sh_name);

		//printf( "section %d type: %d  offset: 0x%X name: %s\n", i,  section->sh_type, section->sh_name, section_name );

		if( kstrcmp( name, section_name ) == 0 ) {
			found_header = section;
			break;
		}
	}

	return found_header;
}

Elf64_Phdr *elf2_new_get_program_header_by_index( elf2_file *f_elf, uint8_t index ) {
	return (Elf64_Phdr*)((uint8_t *)f_elf->file_base + f_elf->header->e_phoff + f_elf->header->e_phentsize*index);
}


Elf64_Shdr *elf2_new_get_section_header_by_index( elf2_file *f_elf, uint8_t index ) {
	return (Elf64_Shdr*)((uint8_t *)f_elf->section_headers + f_elf->header->e_shentsize*(index - 1));
}

Elf64_Shdr* elf2_new_get_section_header_by_type( elf2_file *f_elf, int type ) {

}

Elf64_Sym* elf2_new_get_symbol_entry( elf2_file *f_elf, Elf64_Off s_offset, uint8_t index ) {
	return (Elf64_Sym*)((uint8_t *)f_elf->file_base + s_offset + (sizeof(Elf64_Sym) * index));
}
