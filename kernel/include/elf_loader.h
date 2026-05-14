#if !defined(ELF_LOADER_H_INCLUDED)
#define ELF_LOADER_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <process.h>

int elf_loader_load( process_data *p, uint8_t *data );
int elf_loader_load_binary( process_data *p, uint8_t *data );
void* elf_loader_get_ksym_addr( char *sym_name );
void* elf_loader_dynamic_linker( uint64_t got_table_data, uint8_t got_index );
void linker_test_func( void );

#ifdef __cplusplus
}
#endif

#endif