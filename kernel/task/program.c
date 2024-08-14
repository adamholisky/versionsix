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

int program_load_elf_library( program *p, void *data, size_t size ) {
	debugf( "Loding elf library	: \"%s\"    data: 0x%016llX    size: 0x%llX \n", p->path, data, size );

	for( int i = 0; i < p->elf->num_program_headers; i++ ) {
		Elf64_Phdr *pheader = get_program_header_by_index( p->elf, i );
		if( pheader->p_type == PT_LOAD ) {
			debugf( "Loading +0x%X to 0x%llX for 0x%X (%d) bytes.\n", pheader->p_offset, pheader->p_vaddr, pheader->p_memsz, pheader->p_memsz );

			uint32_t num_pages = pheader->p_memsz / PAGE_SIZE;
			num_pages = num_pages + (pheader->p_memsz % PAGE_SIZE ? 1 : 0);

			program_pages *pages = NULL;

			if( pheader->p_flags & PF_X ) {
				debugf( "    Code segment. %d pages.\n", num_pages );

				p->text_pages = kmalloc( sizeof(program_pages) * num_pages );
				p->num_text_pages = num_pages;

				pages = p->text_pages;
			} else if( pheader->p_flags & PF_R ) {
				debugf( "    Data segment. %d pages.\n", num_pages );

				p->data_pages = kmalloc( sizeof(program_pages) * num_pages );
				p->num_data_pages = num_pages;

				pages = p->data_pages;
			}

			if( pages == NULL ) {
				debugf( "Pages is null. Aborting.\n" );
				return -1;
			}

			debugf( "Allocating %d pages.\n", num_pages );

			for( int j = 0; j < num_pages; j++ ) {
				pages[j].kern_virt = page_allocate_kernel(1);
				pages[j].virt = pheader->p_vaddr + (j * PAGE_SIZE );
				pages[j].phys = paging_virtual_to_physical( pages->kern_virt );

				debugf( "    kern_virt: %X    virt: %X    phys: %X\n", pages[j].kern_virt, pages[j].virt, pages[j].phys );
			}

			memcpy( pages[0].kern_virt, (uint8_t *)data + pheader->p_offset, pheader->p_filesz );
		}
	}
}

int program_load_elf_binary( program *p, void *data, size_t size ) {
	debugf( "Loding elf binary	: \"%s\"    data: 0x%016llX    size: 0x%llX \n", p->path, data, size );
}