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

	program *p = program_allocate();

	strcpy( p->path, path );

	program_load_data( p, data, stats.size );

	kfree(data);
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
}

int program_load_elf_binary( program *p, void *data, size_t size ) {
	debugf( "Loding elf binary	: \"%s\"    data: 0x%016llX    size: 0x%llX \n", p->path, data, size );
}