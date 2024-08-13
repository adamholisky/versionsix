#include <kernel_common.h>
#include <kmemory.h>
#include <task.h>
#include <program.h>
#include <fs.h>
#include <elf.h>

uint16_t id_top;

program *programs;

/**
 * @brief Initalzie the program environment
 * 
 */
void program_init( void ) {
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
int program_load( char *path ) {
	vfs_stat_data stats;

	int stat_error = vfs_stat( vfs_lookup_inode(path), &stats );
	if( stat_error != VFS_ERROR_NONE ) {
		printf( "Error: %d\n", stat_error );
		return 1;
	}

	char *data = kmalloc( stats.size );
	int read_err = vfs_read( vfs_lookup_inode(path), data, stats.size, 0 );
	if( read_err < VFS_ERROR_NONE ) {
		printf( "Error when reading: %d\n", read_err );
		return 1;
	}

	data[ stats.size ] = 0;

	program_load_data( data, stats.size );

	kfree(data);
}

int program_load_data( void *data, size_t size ) {
	uint8_t *data_bytes = (uint8_t *)data;
	
	// Identify the data and run the loading routine
	if( data_bytes[0] == 0x7F ) {
		// Load ELF file

		if( strncmp( data_bytes, "ELF", 3 ) == 0 ) {
			Elf64_Ehdr *elf_header = data;

			switch( elf_header->e_type ) {
				case ET_REL:
					return program_load_elf_module( data, size );
					break;
				case ET_DYN:
					return program_load_elf_library( data, size );
					break;
				case ET_EXEC:
					return program_load_elf_binary( data, size );
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

int program_load_elf_module( void *data, size_t size ) {

}

int program_load_elf_library( void *data, size_t size ) {

}

int program_load_elf_binary( void *data, size_t size ) {

}