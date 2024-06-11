#include <kernel_common.h>
#include <ksymbols.h>
#include <debug.h>
#include <elf.h>

char no_symbol[] = "Cannot find symbol";

extern kinfo kernel_info;

ELF_File *kernel_file;
KernelSymbols *ksyms;

#undef DEBUG_KERNEL_SYMBOLS_INIT
void kernel_symbols_initalize( void ) {
	#ifdef DEBUG_KERNEL_SYMBOLS_INIT
	log_entry_enter();
	#endif

	ksyms = new KernelSymbols();
	kernel_file = new ELF_File( (uint64_t *)kernel_info.kernel_file_address );
	Elf64_Sym* symbol_table = kernel_file->get_symtab();

    for( int i = 0; i < kernel_file->num_symbols; i++ ) {
        Elf64_Sym* sym = (Elf64_Sym*)((uint64_t)symbol_table + sizeof(Elf64_Sym)*i); 
        //debugf( "%03d %s --> 0x%016llx\n", i, kernel_file->get_str_at_offset(sym->st_name), sym->st_value );
		ksyms->add( kernel_file->get_str_at_offset(sym->st_name), sym->st_value, sym->st_size );
    }

	debugf( "Kernel symbols added: %d\n", ksyms->get_total_symbols() );

	#ifdef DEBUG_KERNEL_SYMBOLS_INIT
	log_entry_exit();
	#endif
}

char * kernel_symbols_get_function_at( uint64_t addr ) {
	return ksyms->get_function_at( addr );
}

void KernelSymbols::initalize( void ) {
	this->kernel_symbol_top = 0;
}

void KernelSymbols::add( char * name, uint64_t addr, uint64_t size ) {
	kernel_symbols[ this->kernel_symbol_top ].name = name;
	kernel_symbols[ this->kernel_symbol_top ].addr = addr;
	kernel_symbols[ this->kernel_symbol_top ].size = size;
	this->kernel_symbol_top++;
	//debugf( "0x%03X: 0x%08X + 0x%08X -> %s\n", this->kernel_symbol_top - 1, addr, size, name );
}

KernelSymbol * KernelSymbols::get_symbol_array( void ) {
	return this->kernel_symbols;
}

char * KernelSymbols::get_function_at( uint64_t addr ) {
	char * ret_val = no_symbol;

	for( int i = 0; i<DEBUG_SYMBOLS_MAX; i++ ) {
		if( this->kernel_symbols[i].size == 0 ) continue; 

		if( addr >= this->kernel_symbols[i].addr ) {
			
			if( addr < ( this->kernel_symbols[i].addr + this->kernel_symbols[i].size) ) {
				//klog( "hit: 0x%08X\n", addr );
				ret_val = this->kernel_symbols[i].name;
			}
		}
	}

	return ret_val;
}

KernelSymbol * KernelSymbols::get_symbol( char * name ) {
	KernelSymbol * ret = NULL;

	for( int i = 0; i < this->kernel_symbol_top; i++ ) {
		//klog( "%d", i );
		if( this->kernel_symbols[i].name[0] == 0 ) continue;

		if( strcmp( this->kernel_symbols[i].name, name ) == 0 ) {
			ret = this->kernel_symbols + i;
		}
	}

	return ret;
}

uint64_t KernelSymbols::get_total_symbols( void ) {
	return this->kernel_symbol_top;
}

uint64_t KernelSymbols::get_symbol_addr( char * name ) {
	uint64_t ret = 0;

	KernelSymbol * sym = this->get_symbol( name );

	if( sym != NULL ) {
		ret = sym->addr;
	}

	return ret;
}