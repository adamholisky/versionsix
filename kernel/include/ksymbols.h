#ifndef VIOS_KSYMBOLS_INCLUDED
#define VIOS_KSYMBOLS_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG_SYMBOLS_MAX 4096

#define kdebug_symbol_cannot_find "KDEBUG_cannot_find_symbol"

typedef struct {
	char * 		name;
	uint64_t	addr;
	uint64_t	size;

	#ifdef VIOS_ENABLE_PROFILING
	uint64_t	count;
	uint64_t	start;
	uint64_t	time;
	#endif
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
void symbols_diagnostic( symbol_collection *syms );

void kernel_symbols_initalize( void );
symbol_collection *get_ksyms_object( void );
char *kernel_symbols_get_function_name_at( uint64_t addr );

#ifdef VIOS_ENABLE_PROFILING
void kernel_symbols_inc_count( uint64_t addr );
void kernel_symbols_set_start( uint64_t addr, uint64_t count );
void kernel_symbols_set_time( uint64_t addr, uint64_t count );
#endif

#ifdef __cplusplus
}
#endif
#endif