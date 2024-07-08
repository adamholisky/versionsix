#ifndef VIOS_KSYMBOLS_INCLUDED
#define VIOS_KSYMBOLS_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG_SYMBOLS_MAX 2048

#define kdebug_symbol_cannot_find "KDEBUG_cannot_find_symbol"

typedef struct {
	char * 		name;
	uint64_t	addr;
	uint64_t	size;
} symbol;

typedef struct {
	uint64_t top;
	symbol symbols[ DEBUG_SYMBOLS_MAX ];
} symbol_collection;

void symbols_add( symbol_collection *syms, char * name, uint64_t addr, uint64_t size );
symbol *symbols_get_symbol( symbol_collection *syms, char *name );
symbol *symbols_get_symbol_array( symbol_collection *syms );
uint64_t symbols_get_total_symbols( symbol_collection *syms );
uint64_t symbols_get_symbol_addr( symbol_collection *syms, char * name );
char *symbols_get_function_name_at( symbol_collection *syms, uint64_t addr );
void symbols_initalize( void );

void kernel_symbols_initalize( void );
symbol_collection *get_ksyms_object( void );
char *kernel_symbols_get_function_name_at( uint64_t addr );

#ifdef __cplusplus
}
#endif
#endif