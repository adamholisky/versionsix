#include <kernel_common.h>
#include <kmemory.h>
#include <process.h>
#include <lib/list.h>
#include <vfs.h>

#include <elf.h>
#include <page.h>
#include <ksymbols.h>

#include <elf_loader.h>

#include <lib.h>

extern void elf_dynamic_linker_preamble( void );

int elf_loader_load( process_data* p, uint8_t* data ) {
	//klog( LOG_INFO, "Loding an ELF object: path=%s  data: 0x%016llX  size: 0x%llX", p->path, data, p->exec_size );
	//debugf( "ELF Loader in.\n" );

	if ( data[0] == 0x7F ) {
		Elf64_Ehdr* elf_header = data;

		if ( strncmp( ( data + 1 ), "ELF", 3 ) == 0 ) {
			p->binary_format_data = kmalloc( sizeof( elf_file ) );
			elf_file* elf_f = (elf_file*)p->binary_format_data;

			elf_file_initalize( elf_f, data );

			if ( !elf_load_symbols( elf_f ) ) {
				debugf( "Symbols failed to load." );
				return KERR_ELF_INAVLID;
			}

			//symbols_diagnostic( elf_f->symbols );

			elf_loader_load_binary( p, data );

			p->entry = elf_f->elf_header->e_entry;

			/* switch( elf_header->e_type ) {
				case ET_REL:
					return program_load_elf_module( p, data, size );
					break;
				case ET_DYN:
					return program_load_elf_library( p, data, size );
					break;
				case ET_EXEC:
					return program_load_elf_binary( p, data, size );
					break;
				default:
					debugf( "Invalid ELF e_type: %d\n", elf_header->e_type );
					return ERR_INVALID_ELF_TYPE;
			} */
		}
	}
	else {
		klog( LOG_ERROR, "Program header missing ELF magic" );
		debugf( "Header magic missing.\n" );
	}

	//debugf( "elf loader out\n" );

	return 0;
}

int elf_loader_load_binary( process_data* p, uint8_t* data ) {
	klog( LOG_INFO, "Loading ELF Binary. pid=%d    path=%s    data=0x%016llX", p->pid, p->path, data );

	elf_file* elf_f = (elf_file*)p->binary_format_data;

	uint64_t page_count_from_prev_headers = 0;

	for ( int i = 0; i < elf_f->num_program_headers; i++ ) {
		Elf64_Phdr* pheader = get_program_header_by_index( elf_f, i );

		
		if ( pheader->p_type == PT_LOAD ) {
			//debugf( "New setion to load.\n===============================\n" );
			//debugf( "Loading +0x%X to 0x%llX for 0x%X (%d) bytes.\n", pheader->p_offset, pheader->p_vaddr, pheader->p_memsz, pheader->p_memsz );

			uint32_t num_pages = pheader->p_memsz / PAGE_SIZE;
			num_pages = num_pages + ( pheader->p_memsz % PAGE_SIZE ? 1 : 0 );

			process_exec_section* pages = NULL;

			uint64_t actual_virt_address = 0;
			uint64_t virt_offset = 0;

			if ( pheader->p_vaddr != 0x0 ) {
				// actual_virt_address gets the _programs_ virtual address that we start loading at. We align it to a 4k page. The integer divide forces whole numbers.
				actual_virt_address = ( pheader->p_vaddr / 4096 ) * 4096;

				//num_pages++; // Fix this, should be based off size of program entry + page size to account for an extra page in the vaddr is over page size

				virt_offset = pheader->p_vaddr - actual_virt_address;

				//debugf( "Actual virtual address: 0x%016llX\n", actual_virt_address );
				//debugf( "Virtual offset:         0x%016llx\n", virt_offset );
			}


			if ( pheader->p_flags & PF_X ) {
				//debugf( "    Code segment. %d pages.\n", num_pages );

				p->text_sections = kmalloc( sizeof( process_exec_section ) * num_pages );
				p->text_section_count = num_pages;
				p->text_secton_virt_start = pheader->p_vaddr;

				pages = p->text_sections;
			}
			else if ( pheader->p_flags & PF_R ) {
				//debugf( "    Data segment. %d pages.\n", num_pages );

				p->data_sections = kmalloc( sizeof( process_exec_section ) * num_pages );
				p->data_section_count = num_pages;
				p->data_section_virt_start = pheader->p_vaddr;
				p->data_section_ava = actual_virt_address;

				pages = p->data_sections;
			}

			if ( pages == NULL ) {
				debugf( "Pages is null. Aborting.\n" );
				return -1;
			}

			//debugf( "Allocating %d pages.\n", num_pages );

			for ( int j = 0; j < num_pages; j++ ) {
				pages[j].kern_virt = page_allocate_kernel( 1 );
				pages[j].virt = actual_virt_address + ( j * PAGE_SIZE );
				pages[j].phys = paging_virtual_to_physical( pages[j].kern_virt );

				//debugf( "    kern_virt: 0x%llX    virt: 0x%llX    phys: 0x%llX\n", pages[j].kern_virt, pages[j].virt, pages[j].phys );
			} 
			
			//TODO: START HERE, THIS IS WRONG
			//debugf( "loading to (virt) 0x%X from (phys) 0x%X for 0x%X bytes.\n", virt_offset, pheader->p_offset, pheader->p_filesz );

			//debugf( "loading to (virt) 0x%X from (phys) 0x%X for 0x%X bytes.\n", pheader->p_vaddr, pheader->p_offset, pheader->p_filesz );

			void *kern_virt_data_start = pages[0].kern_virt;
			
			kern_virt_data_start = kern_virt_data_start + virt_offset;

			//debugf( "kern virt data start: 0x%016llX\n", kern_virt_data_start );
			//debugf( "proc virt data start: 0x%016llX\n", pages[0].virt + virt_offset );
			memcpy( kern_virt_data_start, (uint8_t*)data + pheader->p_offset, pheader->p_filesz );

			//kdebug_peek_at_n( kern_virt_data_start, (pheader->p_filesz / 0x10) + 10 );

			page_count_from_prev_headers += num_pages;

			//debugf( "\n===============================\n" );
		}
	}

	Elf64_Shdr* rel_plt = elf_get_section_header_by_name( elf_f, ".rela.plt" );

	// This should be fine, if we don't have it the rest of the processing isn't necessary
	if( rel_plt == NULL ) {
		debugf( "No .rela.plt seectionn present, finishing loading.\n" );
		return 0;
	}

	uint8_t* rel_plt_data = (uint8_t*)data + rel_plt->sh_offset;
	if ( rel_plt != NULL ) {
		
	
#ifdef KDEBUG_PROGRAM_LOAD_ELF_LIBRARY
		debugf( "raw data start: %X\n", data );
		debugf( "plt:sh_offset %X\n", rel_plt->sh_offset );
		debugf( "data %X %x\n", rel_plt_data, *rel_plt_data );
		debugf( ".plt out:\n" );
		
		debugf( "\n\n" );
#endif
	}
	else {
		debugf( "Could not find .rel.plt section.\n" );
	}

	Elf64_Shdr *sect_dynsym = elf_get_section_header_by_name( elf_f, ".dynsym" );
	Elf64_Shdr *sect_dynstr = elf_get_section_header_by_name( elf_f, ".dynstr" );
	p->num_dyn_syms = sect_dynsym->sh_size / sect_dynsym->sh_entsize;
	p->dyn_sym_index = kmalloc( sizeof(symbol_index) * p->num_dyn_syms );

	//debugf( "number dyn_syms: %d\n", p->num_dyn_syms );

	for( int i = 0; i < p->num_dyn_syms; i++ ) {
		Elf64_Sym *sym = (Elf64_Sym *)((uint8_t *)elf_f->file_base + sect_dynsym->sh_offset + (sizeof(Elf64_Sym) * i));
		strcpy( p->dyn_sym_index[i].name, (char *)( (uint8_t *)elf_f->file_base + sect_dynstr->sh_offset + sym->st_name ) );
		p->dyn_sym_index[i].value = sym->st_value;
		p->dyn_sym_index[i].size = sym->st_size;

		//debugf( "dyn_sym[%d] name=%s    value=0x%016llX\n", i, p->dyn_sym_index[i].name, p->dyn_sym_index[i].value );
	}

	Elf64_Shdr *elf_got_section = elf_get_section_header_by_name( elf_f, ".got.plt" );
	if( elf_got_section != NULL ) {
		//uint8_t* got_data = (uint8_t *)p->data_sections[0].kern_virt + elf_got_section->sh_offset;
		uint8_t* got_data = (uint8_t *)p->data_sections[0].kern_virt + elf_got_section->sh_addr - p->data_section_ava ;


		/*debugf( ".got out pre:\n" );
		for ( int j = 0; j < ( elf_got_section->sh_size ); j++ ) {
			debugf_raw( "%02X ", *( got_data + j ) );
		}
		debugf_raw( "\n\n" );*/

		//*(got_data + 0x10) = elf_loader_dynamic_linker;
		uint64_t *got64_t = (uint64_t *)got_data;
		

		/* debugf( "got:             0x%llX\n", got64_t );
		debugf( "got + 0x10:      0x%llX (data: 0x%llX)\n", (got64_t + 1), *(got64_t + 1) );
		debugf( "elfload_dyn_lnk: 0x%llX\n", elf_dynamic_linker_preamble ); */
		
		//*(uint64_t *)(got_data + 0x08) = p->data_sections[0].virt + elf_got_section->sh_offset;
		*(uint64_t *)(got_data + 0x08) = p->text_sections[0].virt + rel_plt->sh_offset;
		*(uint64_t *)(got_data + 0x10) = elf_dynamic_linker_preamble;
		//*(got64_t + 0x3) = elf_loader_dynamic_linker;

		/*debugf( "got + 0x10:      0x%llX (data: 0x%llX)\n", (got64_t + 1), *(got64_t + 1) );

		debugf( ".got out post:\n" );
		for ( int j = 0; j < ( elf_got_section->sh_size ); j++ ) {
			debugf_raw( "%02X ", *( got_data + j ) );
		}
		debugf_raw( "\n\n" );*/
	} else {
		debugf( "Could not locate .got section. Failing hard.\n" );
		do_immediate_shutdown();
	}

	int num_of_rel_plt_entries = ( rel_plt->sh_size / ( sizeof( Elf64_Rela ) ) );
	debugf( "running symbol resolution %d times\n\n", num_of_rel_plt_entries );

	p->rela_sym_index = kmalloc( sizeof(symbol_index) * num_of_rel_plt_entries );

	for ( int rel_num = 0; rel_num < num_of_rel_plt_entries; rel_num++ ) {
		//debugf( "Symbol resolution #%d:\n", rel_num );

		//Elf64_Rel* elf_rel = (Elf64_Rela*)( data + rel_plt->sh_offset + ( rel_num * sizeof( Elf64_Rel ) ) );
		Elf64_Rel* elf_rel = (Elf64_Rela*)( rel_plt_data + ( rel_num * rel_plt->sh_entsize ) );

		//debugf( "  elf_rel->r_info: %llX\n", elf_rel->r_info );
		//debugf( "  elf_rel->r_offset: %llX\n", elf_rel->r_offset );
		//debugf( "  ELF64_R_SYM: %llX\n", ELF64_R_SYM( elf_rel->r_info ) );
		//debugf( "  name: %s\n", elf_get_symbol_name_from_symbol_index( elf_f, ELF64_R_SYM( elf_rel->r_info ) ) );

		//if( elf_get_sym_shndx_from_index((uint32_t*)dl.base, elf_header, ELF32_R_SYM(elf_rel->r_info)) == 0 ) {

		symbol* sym = symbols_get_symbol(
			get_ksyms_object(),
			elf_get_symbol_name_from_symbol_index( elf_f, ELF64_R_SYM( elf_rel->r_info ) )
		);

		p->rela_sym_index[ rel_num ].elf_index = rel_num;
		strcpy( p->rela_sym_index[ rel_num ].name, elf_get_symbol_name_from_symbol_index(elf_f, ELF64_R_SYM( elf_rel->r_info )) );
		p->rela_sym_index[ rel_num ].info = elf_rel->r_info;
		p->rela_sym_index[ rel_num ].offset = elf_rel->r_offset;
	}

}

void* elf_loader_get_ksym_addr( char *sym_name ) {
	void *return_addr = NULL;

	if( sym_name == NULL ) {
		klog( LOG_ERROR, "sym_name is null" );
		return NULL;
	}

	symbol *sym = symbols_get_symbol( get_ksyms_object(), sym_name );

	if( sym != NULL ) {
		return_addr = sym->addr;
	}

	return return_addr;
}

void* elf_loader_dynamic_linker( uint64_t got_table_data, uint8_t got_index ) {
	//debugf( "dynamic linker, yay! got_table_data=0x%016llX    got_index=%02llX\n", got_table_data, got_index );
	//klog( LOG_INFO, "Linking symbol at index %d", got_index );
	void *function_addr = 0;

	process_data *p = process_get_current();

	symbol* sym = symbols_get_symbol( get_ksyms_object(), p->rela_sym_index[got_index].name );

	if( sym != NULL ) {
		function_addr = sym->addr;
	}
	
	if( function_addr == NULL ) {
		for( int i = 0; i < p->num_dyn_syms; i++ ) {
			if( strcmp( p->rela_sym_index[got_index].name , p->dyn_sym_index[i].name ) == 0 ) {
				function_addr = p->dyn_sym_index[i].value;
				break;
			}
		}
	}

	if( function_addr == NULL ) {
		function_addr = lib_dynamic_linker( p->rela_sym_index[got_index].name );
		//klog( LOG_INFO, "Found in library, returning: 0x%016llX\n", function_addr );
	}

	if( function_addr == NULL ) {
		klog( LOG_ERROR, "Returning NULL. got_index=%d pid=%d path=%s", got_index, p->pid, p->path );
	}
	
	return function_addr;
}

void linker_test_func( void ) {
	debugf( "in test func fun!\n" );
}