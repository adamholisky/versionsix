#include <kernel_common.h>
#include <kmemory.h>
#include <task.h>
#include <program.h>
#include <fs.h>
#include <elf.h>
#include <page.h>

uint16_t id_top;

program *programs;

/**
 * @brief Initalzie the program environment
 * 
 */
void program_initalize( void ) {
	id_top = 1000;
	programs = NULL;
}

/**
 * @brief Allocates a new program entry
 * 
 * @return program* 
 */
program *program_allocate( void ) {
	program *new_program = kmalloc( sizeof(program) );
	memset( new_program, 0, sizeof(new_program) );

	new_program->id = id_top++;

	program *top = programs;

	if( top == NULL ) {
		top = new_program;

		return new_program;
	}
	
	bool keep_going = true;

	do {
		if( top->next = NULL ) {
			new_program->prev = top;
			top->next = new_program;
			return new_program;
		}

		top = top->next;
	} while( keep_going );

	return NULL;
}

/**
 * @brief Removes the given program, frees all resources
 * 
 * @param p 
 * @return int 
 */
int program_destroy( program *p ) {
	if( p == NULL ) {
		return -1;
	}

	program *next = p->next;

	if( p->prev != NULL ) {
		p->prev->next = next;
	}
	
	if( p->next != NULL ) {
		p->next = next;
		p->next->prev = p->prev;
	}
	
	// TODO: deallocate pages

	kfree(p);

	return 0;
}

/**
 * @brief Loads the given program
 * 
 * @param path Path to the file to laod
 * @	return int 0 if successfull, otherwise error code
 */	
program *program_load( char *path ) {
	vfs_stat_data stats;

	int stat_error = vfs_stat( vfs_lookup_inode(path), &stats );
	if( stat_error != VFS_ERROR_NONE ) {
		printf( "Error: %d\n", stat_error );
		return NULL;
	}

	char *data = kmalloc( stats.size );
	int read_err = vfs_read( vfs_lookup_inode(path), data, stats.size, 0 );
	if( read_err < VFS_ERROR_NONE ) {
		printf( "Error when reading: %d\n", read_err );
		return NULL;
	}

	data[ stats.size ] = 0;

	program *p = program_allocate();

	strcpy( p->path, path );

	program_load_data( p, data, stats.size );

	kfree(data);

	return p;
}

#undef KDEBUG_PROGRAM_LOAD_DATA
int program_load_data( program *p, void *data, size_t size ) {
	uint8_t *data_bytes = (uint8_t *)data;

	#ifdef KDEBUG_PROGRAM_LOAD_DATA
	debugf( "Loding: \"%s\"    data: 0x%016llX    size: 0x%llX \n", p->path, data, size );
	debugf( "data_bytes[0]: 0x%X\n", data_bytes[0] );	
	#endif
	
	// Identify the data and run the loading routine
	if( data_bytes[0] == 0x7F ) {
		Elf64_Ehdr *elf_header = data;
		// Load ELF file

		if( strncmp( (data_bytes + 1), "ELF", 3 ) == 0 ) {
			p->elf = kmalloc( sizeof(elf_file) );
			elf_file_initalize( p->elf, data );

			if( !elf_load_symbols( p->elf ) ) {
				debugf( "Symbols failed to load." );
				return ERR_ELF_INAVLID;
			}

			symbols_diagnostic( p->elf->symbols );

			switch( elf_header->e_type ) {
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
			}
		}
	} else if( data_bytes[0] = 0xAD ) {
		// Load my own binary format
	}
}

int program_load_elf_module( program *p, void *data, size_t size ) {
	debugf( "Loding elf module	: \"%s\"    data: 0x%016llX    size: 0x%llX \n", p->path, data, size );

	
}

#define KDEBUG_PROGRAM_LOAD_ELF_LIBRARY
int program_load_elf_library( program *p, void *data, size_t size ) {
	debugf( "Loding elf library	: \"%s\"    data: 0x%016llX    size: 0x%llX \n", p->path, data, size );

	for( int i = 0; i < p->elf->num_program_headers; i++ ) {
		Elf64_Phdr *pheader = get_program_header_by_index( p->elf, i );

		
		if( pheader->p_type == PT_LOAD ) {
			debugf( "Loading +0x%X to 0x%llX for 0x%X (%d) bytes.\n", pheader->p_offset, pheader->p_vaddr, pheader->p_memsz, pheader->p_memsz );

			uint32_t num_pages = pheader->p_memsz / PAGE_SIZE;
			num_pages = num_pages + (pheader->p_memsz % PAGE_SIZE ? 1 : 0);

			program_pages *pages = NULL;

			uint64_t actual_virt_address = 0;
			uint64_t virt_offset = 0;

			if( pheader->p_vaddr != 0x0 ) {
				actual_virt_address = (pheader->p_vaddr / 4096) * 4096;

				num_pages++; // Fix this, should be based off size of program entry + page size to account for an extra page in the vaddr is over page size

				virt_offset = pheader->p_vaddr - actual_virt_address;
			}


			if( pheader->p_flags & PF_X ) {
				debugf( "    Code segment. %d pages.\n", num_pages );

				p->text_pages = kmalloc( sizeof(program_pages) * num_pages );
				p->num_text_pages = num_pages;
				p->text_pages_virt_start = pheader->p_vaddr;

				pages = p->text_pages;
			} else if( pheader->p_flags & PF_R ) {
				debugf( "    Data segment. %d pages.\n", num_pages );

				p->data_pages = kmalloc( sizeof(program_pages) * num_pages );
				p->num_data_pages = num_pages;
				p->data_pages_virt_start = pheader->p_vaddr;

				pages = p->data_pages;
			}

			if( pages == NULL ) {
				debugf( "Pages is null. Aborting.\n" );
				return -1;
			}

			debugf( "Allocating %d pages.\n", num_pages );

			for( int j = 0; j < num_pages; j++ ) {
				pages[j].kern_virt = page_allocate_kernel(1);
				pages[j].virt = actual_virt_address + (j * PAGE_SIZE );
				pages[j].phys = paging_virtual_to_physical( pages[j].kern_virt );

				debugf( "    kern_virt: %X    virt: %X    phys: %X\n", pages[j].kern_virt, pages[j].virt, pages[j].phys );
			}

			memcpy( pages[0].kern_virt + virt_offset, (uint8_t *)data + pheader->p_offset, pheader->p_filesz );
		}
	}

	Elf64_Shdr *rel_plt = elf_get_section_header_by_name( p->elf, ".rela.plt" );
	if (rel_plt != NULL) {
		uint8_t *rel_plt_data = (uint8_t*)data + rel_plt->sh_offset;

		#ifdef KDEBUG_PROGRAM_LOAD_ELF_LIBRARY
		debugf( "raw data start: %X\n", data );
		debugf( "plt:sh_offset %X\n", rel_plt->sh_offset);
		debugf( "data %X %x\n", rel_plt_data, *rel_plt_data );
		debugf( ".plt out:\n");
		for (int j = 0; j < (rel_plt->sh_size); j++) {
			debugf_raw("%02X ", *(rel_plt_data + j));
		}
		debugf("\n\n");
		#endif
	}
	else {
		debugf("Could not find .rel.plt section.\n");
	}

	Elf64_Shdr* got_plt = elf_get_section_header_by_name( p->elf, ".got.plt" );
	if (got_plt != NULL) {
		uint8_t *got_plt_data = (uint8_t *)data + got_plt->sh_offset;

		#ifdef KDEBUG_PROGRAM_LOAD_ELF_LIBRARY
		debugf(".got.plt out:\n");
		for (int j = 0; j < (got_plt->sh_size); j++) {
			debugf_raw("%02X ", *(got_plt_data + j));
		}
		debugf("\n\n");
		#endif
	}
	else {
		debugf("Could not find .got.plt section\n");
	}

	for(int rel_num = 0; rel_num < (rel_plt->sh_size/(sizeof(Elf64_Rela))); rel_num++) {
		Elf64_Rel *elf_rel = (Elf64_Rela*)((uint8_t*)data + rel_plt->sh_offset + (rel_num * sizeof(Elf64_Rel)));

		debugf( "elf_rel->r_info: %llX\n", elf_rel->r_info );
		debugf( "elf_rel->r_offset: %llX\n", elf_rel->r_offset );
		debugf( "ELF64_R_SYM: %llX\n", ELF64_R_SYM( elf_rel->r_info ) );

		//if( elf_get_sym_shndx_from_index((uint32_t*)dl.base, elf_header, ELF32_R_SYM(elf_rel->r_info)) == 0 ) {

			symbol *sym = symbols_get_symbol_addr( 
												get_ksyms_object(),
												elf_get_symbol_name_from_symbol_index( p->elf, ELF64_R_SYM( elf_rel->r_info ) )
												);
			
			if( sym == NULL ) {
				debugf( "Symbol not found.\n" );
				return;
			}

			debugf( "Found symbol: %s at %llX\n", elf_get_symbol_name_from_symbol_index( p->elf, ELF64_R_SYM( elf_rel->r_info ) ), sym->addr );

			uint8_t *data_pages_start = (uint8_t *)p->data_pages[0].kern_virt;
			uint64_t *got_entry = (uint64_t*)(data_pages_start + (elf_rel->r_offset - p->data_pages_virt_start) );
			
			debugf( "got entry: %llx\n", got_entry );

			*got_entry = sym->addr;

			//*got_entry = (uint64_t)kdebug_get_symbol_addr( elf_get_sym_name_from_index((uint32_t*)dl.base, elf_header, ELF32_R_SYM(elf_rel->r_info)) );

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

int program_load_elf_binary( program *p, void *data, size_t size ) {
	debugf( "Loding elf binary	: \"%s\"    data: 0x%016llX    size: 0x%llX \n", p->path, data, size );
}