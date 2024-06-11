#ifndef VIOS_KSYMBOLS_INCLUDED
#define VIOS_KSYMBOLS_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG_SYMBOLS_MAX 2048

class KernelSymbol {
	public:
		char * 		name;
		uint64_t	addr;
		uint64_t	size;
};

class KernelSymbols {
	private:
		uint64_t kernel_symbol_top;
		KernelSymbol kernel_symbols[ DEBUG_SYMBOLS_MAX ];
		
	public:
		const char * kdebug_symbol_cannot_find = "KDEBUG_cannot_find_symbol";

		void add( char * name, uint64_t addr, uint64_t size );
		KernelSymbol * get_symbol_array( void );
		KernelSymbol * get_symbol( char * name );
		uint64_t get_total_symbols( void );
		uint64_t get_symbol_addr( char * name );
		char * get_function_at( uint64_t addr );
		void initalize( void );
};

void kernel_symbols_initalize( void );

#ifdef __cplusplus
}
#endif
#endif