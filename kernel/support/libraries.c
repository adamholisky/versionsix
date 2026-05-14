#include <kernel_common.h>
#include <kmemory.h>
#include <lib/list.h>
#include <vfs.h>
#include <elf2.h>
#include <page.h>
#include <elf_loader.h>

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
	memset( lib->f_elf, 0, sizeof(elf2_file) );
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

	// Process relocation tables
	lib_load_relocation_tables( lib, lib_data );

	// Attach to shared library list
	avs_list_append( lib_registry, lib );

	return 0;
}

/**
 * @brief Processes the .rela.plt and .rela.dyn sections for functions, assigns correct addresses/offsets
 * 
 * @param lib 
 * @param lib_data 
 * @return int 
 */
int lib_load_relocation_tables( lib_shared *lib, void *lib_data ) {
	// First the .rela.dyn section

	Elf64_Shdr *sh_dyn = elf2_new_get_section_header_by_name( lib->f_elf, ".rela.dyn" );
	if( sh_dyn != NULL ) {
		int num_entries = sh_dyn->sh_size / sizeof(Elf64_Rela);

		Elf64_Rela *dyn_ents = (Elf64_Rela *)((uint8_t*)lib_data + sh_dyn->sh_offset );

		for( int i = 0; i < num_entries; i++ ) {
			if( ELF64_R_TYPE( dyn_ents[i].r_info ) == R_X86_64_GLOB_DAT ) {
				*(uint64_t *)(lib->lib_base + dyn_ents[i].r_offset) = lib->lib_base + lib->f_elf->dynsym_ents[ ELF64_R_SYM(dyn_ents[i].r_info) ].addr;
			}
		}
	}

	Elf64_Shdr *sh_plt = elf2_new_get_section_header_by_name( lib->f_elf, ".rela.plt" );
	if( sh_plt != NULL ) {
		int num_entries = sh_plt->sh_size / sizeof(Elf64_Rela);

		Elf64_Rela *dyn_ents = (Elf64_Rela *)((uint8_t*)lib_data + sh_plt->sh_offset );

		for( int i = 0; i < num_entries; i++ ) {
			if( ELF64_R_TYPE( dyn_ents[i].r_info ) == R_X86_64_JUMP_SLOT ) {
				void *addr = elf_loader_get_ksym_addr( lib->f_elf->dynsym_ents[ELF64_R_SYM(dyn_ents[i].r_info)].name );
				
				if( addr == NULL ) {
					klog( LOG_ERROR, "got null addr for %s", lib->f_elf->dynsym_ents[ELF64_R_SYM(dyn_ents[i].r_info)].name );
				} else {
					*(uint64_t *)(lib->lib_base + dyn_ents[i].r_offset) = addr;
				}
			}
		}
	}
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

		//lib->lib_symbols[lib_sym_count].addr = lib->mem_sections[0].pages[0].virt + lib->f_elf->dynsym_ents[i].addr;
		lib->lib_symbols[lib_sym_count].addr = lib->lib_base + lib->f_elf->dynsym_ents[i].addr;
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

	int i_adjusted = 0;

	int virt_mem_max = 0;

	for( int i = 0; i < lib->f_elf->program_headers_num_ents; i++ ) {
		if( program_headers[i].type != PT_LOAD ) {
			//printf( "Skipping %d\n", i );
			continue;
		}

		uint64_t ent_end_addr = program_headers[i].virt_addr + program_headers[i].virt_size;
		
		if( ent_end_addr > virt_mem_max ) {
			virt_mem_max = ent_end_addr;
		}
	}

	lib->total_pages = (virt_mem_max / PAGE_SIZE) + 1;

	klog( LOG_DEBUG, "virt_mem_max = 0x%X    total_pages = %d", virt_mem_max, lib->total_pages );

	lib->lib_base = page_allocate_kernel( lib->total_pages );
	//page_map( lib->lib_base, paging_virtual_to_physical(lib->lib_base) );
	memset( lib->lib_base, 0, lib->total_pages * PAGE_SIZE );

	for( int i = 0; i < lib->f_elf->program_headers_num_ents; i++ ) {
		if( program_headers[i].type != PT_LOAD ) {
			continue;
		}

		memcpy( (uint8_t *)lib->lib_base + program_headers[i].virt_addr, (uint8_t *)lib_data + program_headers[i].phys_offset, program_headers[i].phys_size );
	}
}

/**
 * @brief Return the symbol's address from any library that's been loaded.
 * 
 * @param name 
 * @return void* 
 */
void *lib_dynamic_linker( char *name ) {
	avs_node *n = lib_registry->head;

	for( int i = 0; i < lib_registry->size; i++ ) {
		lib_shared *lib = (lib_shared *)n->data;

		for( int j = 0; j < lib->num_symbols; j++ ) {
			if( kstrcmp( lib->lib_symbols[j].name, name ) == 0 ) {				
				return lib->lib_symbols[j].addr;
			}
		}

		n = n->next;
	}
}