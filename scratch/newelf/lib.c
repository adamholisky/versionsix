#include "elfold.h"
#include "elf.h"
#include "lib.h"

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>



// Start move to lib.h

#include "list.h"

typedef struct {
	char * 		name;
	uint64_t	addr;
	uint64_t	size;

	#ifdef VIOS_ENABLE_PROFILING
	uint64_t	count;
	uint64_t	start;
	uint64_t	time;
	#endif
} lib_symbol;

typedef struct {
	char name[50];
	uint64_t version;
	avs_list *lib_symbols;
	elf_new_file *f_elf;

	

} lib_shared;

int lib_register( char *pathname );
void lib_initalize( void );
int lib_load_symbols( lib_shared *lib, void *lib_data );

// End move to lib.h

int main( int argc, char *argv[] ) {
	lib_initalize();
	
	lib_register( "./avsul.so" );

	return 0;
}

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
	/* file_stats st;
	int attr_err = vfs_getattr( pathname, &st );
	if( attr_err != VFS_ERROR_NONE ) {
		//klog( LOG_PANIC, "Cannot load library. pathname=%s  err=%d", pathname, attr_err );
		printf( "Can't load lib\n" );
		return KERROR_UNKNOWN;
	} */

	struct stat st;
	int stat_ret = stat( pathname, &st );

	printf( "lib size: %d", st.st_size );

	FILE *f_lib = fopen( pathname, "r" );
	if( f_lib == NULL ) {
		printf( "Can't open.\n" );
		return KERROR_UNKNOWN;
	}

	void *lib_data = malloc( st.st_size );
	size_t read_ret = fread( lib_data, st.st_size, 1, f_lib );

	/* void *lib_data = kmalloc( st.st_size );
	int read_err = vfs_read( pathname, lib_data, st.st_size, 0 );
	if( read_err != VFS_ERROR_NONE ) {
		//klog( LOG_ERROR, "Lib loaded %d bytes, expected %d", read_err, st.st_size );
		printf( "Read error on library\n" );
		return KERROR_UNKNOWN;
	} */

	// Allocate the library info
	lib_shared *lib = malloc( sizeof(lib_shared) );
	lib->f_elf = malloc( sizeof(elf_new_file) );
	int elf_err = elf_new_initalize_file( lib->f_elf, lib_data );
	if( elf_err != KERROR_NONE ) {
		//klog( LOG_ERROR, "ELF file init failed: %d", elf_err );
		printf( "Elf init failed.\n" );
		return KERROR_UNKNOWN;
	}

	// Load the symbols
	lib_load_symbols( lib, lib_data );

	// Attach to shared library list
	avs_list_append( lib_registry, lib );

	return 0;
}

int lib_load_symbols( lib_shared *lib, void *lib_data ) {
	KASSERT_NOT_NULL_R( lib );
	KASSERT_NOT_NULL_R( lib_data );

	Elf64_Shdr *sh_dynsym = elf_new_get_section_header_by_name( lib->f_elf, "dynsym" );

	if( sh_dynsym == NULL ) {
		//klog( LOG_ERROR, "Could not find dynsym section." );
		printf( "Can't find dynsym section.\n" );
		return KERROR_UNKNOWN;
	}

	
}