#include <kernel_common.h>
#include <kmemory.h>
#include <process.h>
#include <lib/list.h>
#include <vfs.h>

#include <elf.h>
#include <page.h>
#include <ksymbols.h>

#include <elf_loader.h>

int elf_loader_load( process_data* p, uint8_t* data ) {
	klog( LOG_INFO, "Loding an ELF object: path=%s    data: 0x%016llX    size: 0x%llX \n", p->path, data, p->exec_size );

	if ( data[0] == 0x7F ) {
		Elf64_Ehdr* elf_header = data;

		if ( strncmp( ( data + 1 ), "ELF", 3 ) == 0 ) {
			p->binary_format_data = kmalloc( sizeof( elf_file ) );
			elf_file* elf_f = (elf_file*)p->binary_format_data;

			elf_file_initalize( elf_f, data );

			if ( !elf_load_symbols( elf_f ) ) {
				debugf( "Symbols failed to load." );
				return ERR_ELF_INAVLID;
			}

			symbols_diagnostic( elf_f->symbols );

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
	}

	
}

int elf_loader_load_binary( process_data* p, uint8_t* data ) {
	klog( LOG_INFO, "Loading ELF Binary. pid=%d    path=%s    data=0x%016llX", p->pid, p->path, data );

	elf_file* elf_f = (elf_file*)p->binary_format_data;

	for ( int i = 0; i < elf_f->num_program_headers; i++ ) {
		Elf64_Phdr* pheader = get_program_header_by_index( elf_f, i );


		if ( pheader->p_type == PT_LOAD ) {
			debugf( "Loading +0x%X to 0x%llX for 0x%X (%d) bytes.\n", pheader->p_offset, pheader->p_vaddr, pheader->p_memsz, pheader->p_memsz );

			uint32_t num_pages = pheader->p_memsz / PAGE_SIZE;
			num_pages = num_pages + ( pheader->p_memsz % PAGE_SIZE ? 1 : 0 );

			process_exec_section* pages = NULL;

			uint64_t actual_virt_address = 0;
			uint64_t virt_offset = 0;

			if ( pheader->p_vaddr != 0x0 ) {
				actual_virt_address = ( pheader->p_vaddr / 4096 ) * 4096;

				num_pages++; // Fix this, should be based off size of program entry + page size to account for an extra page in the vaddr is over page size

				virt_offset = pheader->p_vaddr - actual_virt_address;
			}


			if ( pheader->p_flags & PF_X ) {
				debugf( "    Code segment. %d pages.\n", num_pages );

				p->text_sections = kmalloc( sizeof( process_exec_section ) * num_pages );
				p->text_section_count = num_pages;
				p->text_secton_virt_start = pheader->p_vaddr;

				pages = p->text_sections;
			}
			else if ( pheader->p_flags & PF_R ) {
				debugf( "    Data segment. %d pages.\n", num_pages );

				p->data_sections = kmalloc( sizeof( process_exec_section ) * num_pages );
				p->data_section_count = num_pages;
				p->data_section_virt_start = pheader->p_vaddr;

				pages = p->data_sections;
			}

			if ( pages == NULL ) {
				debugf( "Pages is null. Aborting.\n" );
				return -1;
			}

			debugf( "Allocating %d pages.\n", num_pages );

			for ( int j = 0; j < num_pages; j++ ) {
				pages[j].kern_virt = page_allocate_kernel( 1 );
				pages[j].virt = actual_virt_address + ( j * PAGE_SIZE );
				pages[j].phys = paging_virtual_to_physical( pages[j].kern_virt );

				debugf( "    kern_virt: %X    virt: %X    phys: %X\n", pages[j].kern_virt, pages[j].virt, pages[j].phys );
			}
			
			//TODO: START HERE, THIS IS WRONG
			memcpy( pages[0].kern_virt + virt_offset, (uint8_t*)data + pheader->p_offset, pheader->p_filesz );
		}
	}

	Elf64_Shdr* rel_plt = elf_get_section_header_by_name( elf_f, ".rela.plt" );
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

	Elf64_Shdr* got_plt = elf_get_section_header_by_name( elf_f, ".got.plt" );
	if ( got_plt != NULL ) {
		uint8_t* got_plt_data = (uint8_t*)data + got_plt->sh_offset;

#ifdef KDEBUG_PROGRAM_LOAD_ELF_LIBRARY
		debugf( ".got.plt out:\n" );
		for ( int j = 0; j < ( got_plt->sh_size ); j++ ) {
			debugf_raw( "%02X ", *( got_plt_data + j ) );
		}
		debugf( "\n\n" );
#endif
	}
	else {
		debugf( "Could not find .got.plt section\n" );
	}

	debugf( "\n" );
	int num_of_rel_plt_entries = ( rel_plt->sh_size / ( sizeof( Elf64_Rela ) ) );
	debugf( "running symbol resolution %d times\n\n", num_of_rel_plt_entries );

	for ( int rel_num = 0; rel_num < num_of_rel_plt_entries; rel_num++ ) {
		debugf( "Symbol resolution #%d:\n", rel_num );

		//Elf64_Rel* elf_rel = (Elf64_Rela*)( data + rel_plt->sh_offset + ( rel_num * sizeof( Elf64_Rel ) ) );
		Elf64_Rel* elf_rel = (Elf64_Rela*)( rel_plt_data + ( rel_num * rel_plt->sh_entsize ) );

		debugf( "  elf_rel->r_info: %llX\n", elf_rel->r_info );
		debugf( "  elf_rel->r_offset: %llX\n", elf_rel->r_offset );
		debugf( "  ELF64_R_SYM: %llX\n", ELF64_R_SYM( elf_rel->r_info ) );
		debugf( "  name: %s\n", elf_get_symbol_name_from_symbol_index( elf_f, ELF64_R_SYM( elf_rel->r_info ) ) );

		//if( elf_get_sym_shndx_from_index((uint32_t*)dl.base, elf_header, ELF32_R_SYM(elf_rel->r_info)) == 0 ) {

		symbol* sym = symbols_get_symbol(
			get_ksyms_object(),
			elf_get_symbol_name_from_symbol_index( elf_f, ELF64_R_SYM( elf_rel->r_info ) )
		);

		if ( sym == NULL ) {
			debugf( "  Symbol not found, searching local...\n" );

			uint64_t local_sym_addr = elf_get_symbol_addr_from_symbol_name( elf_f, elf_get_symbol_name_from_symbol_index(elf_f, ELF64_R_SYM( elf_rel->r_info)) );

			if( local_sym_addr != 0 ) {
				debugf( "  found local symbol with an address, using it: 0x%X\n", local_sym_addr );

				uint8_t* data_pages_start = (uint8_t*)p->data_sections[0].kern_virt;
				uint64_t* got_entry = (uint64_t*)( data_pages_start + elf_rel->r_offset - p->data_sections[0].virt );

				debugf( "  got entry pre:  %llx\n", *got_entry );

				*got_entry = local_sym_addr;

				debugf( "  got entry post: %llx\n", *got_entry );
			} else {
				debugf( "  Local symbol not found. Big fail?\n" );
			}
		} else {
			// Symbol is a kernel symbol, use the kernel symbol table
			debugf( "  ksym: name: \"%s\"    addr: 0x%016llX\n", sym->name, sym->addr );

			debugf( "  Found symbol: %s at %llX\n", elf_get_symbol_name_from_symbol_index( elf_f, ELF64_R_SYM( elf_rel->r_info ) ), sym->addr );

			uint8_t* data_pages_start = (uint8_t*)p->data_sections[0].kern_virt;
			debugf( "  0x%016llx\n", data_pages_start + elf_rel->r_offset - p->data_sections[0].virt );
			uint64_t* got_entry = (uint64_t*)( data_pages_start + elf_rel->r_offset - p->data_sections[0].virt );

			debugf( "  got entry pre:  %llx\n", *got_entry );

			*got_entry = sym->addr;

			debugf( "  got entry post: %llx\n", *got_entry );

			//*got_entry = (uint64_t)kdebug_get_symbol_addr( elf_get_sym_name_from_index((uint32_t*)dl.base, elf_header, ELF32_R_SYM(elf_rel->r_info)) );
		}

		

#ifdef KDEBUG_PROGRAM_LOAD_ELF_LIBRARY
		debugf( "GOT entry: 0x%llX\n", *got_entry );
		//debugf( "rel sym: 0x%08X, %d, %d, %X, %s\n", elf_rel->r_offset, ELF32_R_TYPE(elf_rel->r_info), ELF32_R_SYM(elf_rel->r_info),  elf_get_sym_shndx_from_index((uint32_t*)dl.base, elf_header, ELF32_R_SYM(elf_rel->r_info)), elf_get_sym_name_from_index((uint32_t*)dl.base, elf_header, ELF32_R_SYM(elf_rel->r_info)) );
#endif
/* } else {
	klog( "Should not go here.\n" );
	// Link main -- I think I'm doing something wrong by having to do this, maybe not handling got right?
	uint32_t *got_entry = (uint32_t*)(dl.base + elf_rel->r_offset);

	*got_entry = (uint32_t)elf_get_sym_value_from_index((uint32_t*)dl.base, elf_header, ELF32_R_SYM(elf_rel->r_info));
} */
	}

}