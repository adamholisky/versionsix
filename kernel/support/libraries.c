#include <kernel_common.h>
#include <kmemory.h>
#include <lib/list.h>
#include <vfs.h>
#include <elf2.h>
#include <page.h>

#include <lib.h>

extern void elf_dynamic_linker_preamble( void );

/**
 * General idea:
 *
 * 1. Get library size from so's header
 * 2. Load lib's symbols into its symbol store object
 * 3. When dynamic linker is looking for routines, use the lib's symbol store object, transition to code accordingly
 */

avs_list* lib_registry;

void lib_initalize( void ) {
	lib_registry = avs_list_init();
}

int lib_register( char *pathname ) {
	KASSERT_NOT_NULL_R( pathname );

	// Load the file
	file_stats st;
	int attr_err = vfs_getattr( pathname, &st );
	if( attr_err != VFS_ERROR_NONE ) {
		klog( LOG_PANIC, "Cannot load library. pathname=%s  err=%d", pathname, attr_err );
		return KERR_UNKNOWN;
	}

	klog( LOG_INFO, "lib size: %d", st.st_size );

	kA();

	void *lib_data = kmalloc( st.st_size );
	int read_err = vfs_read( pathname, lib_data, st.st_size, 0 );
	if( read_err != VFS_ERROR_NONE ) {
		klog( LOG_ERROR, "Lib loaded %d bytes, expected %d", read_err, st.st_size );
		return KERR_UNKNOWN;
	}

	kB();

	// Allocate the library info
	lib_shared *lib = kmalloc( sizeof(lib_shared) );
	memset( lib, 0, sizeof(lib_shared) );
	lib->f_elf = kmalloc( sizeof(elf2_file) );
	memset( lib->f_elf, 0, sizeof(lib_shared) );
	int elf_err = elf2_new_initalize_file( lib->f_elf, lib_data );
	if( elf_err != KERR_NONE ) {
		klog( LOG_ERROR, "ELF file init failed: %d", elf_err );
		printf( "Elf init failed.\n" );
		return KERR_UNKNOWN;
	}

	// Bring in the program headers and all the data
	lib_load_program_headers( lib, lib_data );

	// Load the symbols
	lib_load_symbols( lib, lib_data );

	// Setup the linker
	lib_load_dynamic_linker( lib );

	// Attach to shared library list
	avs_list_append( lib_registry, lib );

	return 0;
}

/**
 * @brief Load the symbols from the library for use
 * 
 * @param lib 
 * @param lib_data 
 * @return int 
 */
int lib_load_symbols( lib_shared *lib, void *lib_data ) {
	KASSERT_NOT_NULL_R( lib );
	KASSERT_NOT_NULL_R( lib_data );

	if( !lib->f_elf->has_dynsym ) {
		printf( "Dynsym bool not set\n" );
		return 0;
	}

	lib->num_symbols = lib->f_elf->num_symbols;
	lib->lib_symbols = kmalloc( sizeof(lib_symbol) * lib->num_symbols );

	printf( "num syms: %d\n", lib->num_symbols );

	int lib_sym_count = 0;

	for( int i = 0; i < lib->f_elf->dynsym_num_ents; i++ ) {
		if( lib->f_elf->dynsym_ents[i].sh_index == 0 ) { 
			//printf( "Skipping symbol %d\n", i );
			continue;
		}

		lib->lib_symbols[lib_sym_count].addr = lib->mem_sections[0].pages[0].virt + lib->f_elf->dynsym_ents[i].addr;
		kstrcpy( lib->lib_symbols[lib_sym_count].name, lib->f_elf->dynsym_ents[i].name );
		

		printf( "Symbol %s lives at 0x%016llX\n", lib->lib_symbols[lib_sym_count].name, lib->lib_symbols[lib_sym_count].addr );

		lib_sym_count++;
	}

	//lib->num_symbols = lib_sym_count;
}

/**
 * @brief Allocate and populate the data from the program headers in the elf so
 * 
 * @param lib 
 * @param lib_data 
 * @return int 
 */
int lib_load_program_headers( lib_shared *lib, void *lib_data ) {
	if( !lib->f_elf->has_program_headers ) {
		return 0;
	}

	lib->num_mem_sections = lib->f_elf->program_headers_num_loads;
	lib->mem_sections = kmalloc( sizeof(lib_memory_section) * lib->num_mem_sections );

	elf2_program_header *program_headers = lib->f_elf->program_headers;

	// Populate the pages array
	for( int i = 0; i < lib->f_elf->program_headers_num_ents; i++ ) {
		if( program_headers[i].type != PT_LOAD ) {
			//printf( "Skipping %d\n", i );
			continue;
		}

		lib->mem_sections[i].num_pages = (program_headers[i].virt_size / PAGE_SIZE) + 1;
		lib->mem_sections[i].read = program_headers[i].read;
		lib->mem_sections[i].write = program_headers[i].write;
		lib->mem_sections[i].execute = program_headers[i].execute;
		lib->mem_sections[i].pages = kmalloc( sizeof(lib_shared_page) * lib->mem_sections[i].num_pages );

		for( int j = 0; j < lib->mem_sections[i].num_pages; j++ ) {			
			lib->mem_sections[i].pages[j].virt = page_allocate_kernel( 1 );
			lib->mem_sections[i].pages[j].phys = paging_virtual_to_physical( lib->mem_sections[i].pages[j].virt );

			memset( lib->mem_sections[i].pages[j].virt, 0, PAGE_SIZE );

			page_map( lib->mem_sections[i].pages[j].virt, lib->mem_sections[i].pages[j].phys );

			printf( "Page allocated. virt=0x%016llX\n", lib->mem_sections[i].pages[j].virt );
		}

		printf( "mem copy to 0x%016llX from 0x%016llX for %X\n", lib->mem_sections[i].pages[0].virt, (uint8_t *)lib_data + program_headers[i].phys_addr, program_headers[i].phys_size );

		memcpy( lib->mem_sections[i].pages[0].virt, (uint8_t *)lib_data + program_headers[i].phys_addr, program_headers[i].phys_size );
	}
}

void lib_load_dynamic_linker( lib_shared *lib ) {
	Elf64_Shdr *elf_got_section = elf2_new_get_section_header_by_name( lib->f_elf, ".got.plt" );
	if( elf_got_section != NULL ) {
		uint8_t *got_data = lib->mem_sections[0].pages[0].virt + elf_got_section->sh_addr;

		uint64_t *got64_t = (uint64_t *)got_data;

		//*(uint64_t *)(got_data + 0x08) = p->text_sections[0].virt + rel_plt->sh_offset;
		*(uint64_t *)(got_data + 0x10) = elf_dynamic_linker_preamble;
	} else {
		debugf( "Could not locate .got section. Failing hard.\n" );
		do_immediate_shutdown();
	}
}

void *lib_dynamic_linker( char *name ) {
	avs_node *n = lib_registry->head;

	printf( "looking for %s\n", name );

	for( int i = 0; i < lib_registry->size; i++ ) {
		lib_shared *lib = (lib_shared *)n->data;

		printf( "a\n" );

		for( int j = 0; j < lib->num_symbols; j++ ) {
			printf( "b: %s\n", lib->lib_symbols[j].name );
			if( kstrcmp( lib->lib_symbols[j].name, name ) == 0 ) {
				printf( "Returning 0x%016llX\n", lib->lib_symbols[j].addr );
				
				return lib->lib_symbols[j].addr;
			}
		}
	}
}