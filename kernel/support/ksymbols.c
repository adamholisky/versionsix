#include <kernel_common.h>
#include <ksymbols.h>
#include <debug.h>
#include <elf.h>

char no_symbol[] = "Cannot find symbol";

extern kinfo kernel_info;

elf_file kernel_file;
symbol_collection ksyms;

#define DEBUG_KERNEL_SYMBOLS_INIT
void kernel_symbols_initalize( void ) {
	#ifdef DEBUG_KERNEL_SYMBOLS_INIT
	log_entry_enter();
	#endif

	ksyms.top = 0;

	elf_file_initalize( &kernel_file, (uint64_t *)kernel_info.kernel_file_address );
	Elf64_Sym* symbol_table = elf_get_symtab( &kernel_file );

    for( int i = 0; i < kernel_file.num_symbols; i++ ) {
        Elf64_Sym* sym = (Elf64_Sym*)((uint64_t)symbol_table + sizeof(Elf64_Sym)*i); 
        //debugf( "%03d %s --> 0x%016llx\n", i, kernel_file->get_str_at_offset(sym->st_name), sym->st_value );
		symbols_add( &ksyms, elf_get_str_at_offset( &kernel_file, sym->st_name), sym->st_value, sym->st_size );
    }

	debugf( "Kernel symbols added: %d\n", symbols_get_total_symbols( &ksyms ) );

	#ifdef DEBUG_KERNEL_SYMBOLS_INIT
	log_entry_exit();
	#endif
}

symbol_collection *get_ksyms_object( void ) {
	return &ksyms;
}

char *kernel_symbols_get_function_name_at( uint64_t addr ) {
	return symbols_get_function_name_at( &ksyms, addr );
}

#ifdef VIOS_ENABLE_PROFILING
void kernel_symbols_inc_count( uint64_t addr ) {
	for( int i = 0; i < ksyms.top; i++ ) {
		if( ksyms.symbols[i].addr == addr ) {
			ksyms.symbols[i].count++;
			return;
		}
	}
}

void kernel_symbols_set_start( uint64_t addr, uint64_t count ) {
	for( int i = 0; i < ksyms.top; i++ ) {
		if( ksyms.symbols[i].addr == addr ) {
			ksyms.symbols[i].start = count;
			return;
		}
	}
}

void kernel_symbols_set_time( uint64_t addr, uint64_t count ) {
	for( int i = 0; i < ksyms.top; i++ ) {
		if( ksyms.symbols[i].addr == addr ) {
			ksyms.symbols[i].time = ksyms.symbols[i].time + (count - ksyms.symbols[i].start);
			ksyms.symbols[i].start = 0;
			return;
		}
	}
}
#endif

symbol *symbols_get_symbol_array( symbol_collection *syms ) {
	return syms->symbols;
}

void symbols_add( symbol_collection *syms, char *name, uint64_t addr, uint64_t size ) {
	syms->symbols[ syms->top ].name = name;
	syms->symbols[ syms->top ].addr = addr;
	syms->symbols[ syms->top ].size = size;
	syms->top++;
	//debugf( "0x%03X: 0x%08X + 0x%08X -> %s\n", this->kernel_symbol_top - 1, addr, size, name );
}

char *symbols_get_function_name_at( symbol_collection *syms, uint64_t addr ) {
	char * ret_val = no_symbol;

	for( int i = 0; i<DEBUG_SYMBOLS_MAX; i++ ) {
		if( syms->symbols[i].size == 0 ) continue; 

		if( addr >= syms->symbols[i].addr ) {
			
			if( addr < ( syms->symbols[i].addr + syms->symbols[i].size) ) {
				//klog( "hit: 0x%08X\n", addr );
				ret_val = syms->symbols[i].name;
			}
		}
	}

	return ret_val;
}

symbol *symbol_get_symbol( symbol_collection *syms, char *name ) {
	symbol *ret = NULL;

	for( int i = 0; i < syms->top; i++ ) {
		//klog( "%d", i );
		if( syms->symbols[i].name[0] == 0 ) continue;

		if( strcmp( syms->symbols[i].name, name ) == 0 ) {
			ret = &syms->symbols[i];
		}
	}

	return ret;
}

uint64_t symbols_get_total_symbols( symbol_collection *syms ) {
	return syms->top;
}

uint64_t symbols_get_symbol_addr( symbol_collection *syms, char *name ) {
	uint64_t ret = 0;

	symbol * sym = symbol_get_symbol( syms, name );

	if( sym != NULL ) {
		ret = sym->addr;
	}

	return ret;
}