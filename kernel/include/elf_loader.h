#if !defined(ELF_LOADER_H_INCLUDED)
#define ELF_LOADER_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

int elf_loader_load( process_data *p, uint8_t *data );
int elf_loader_load_binary( process_data *p, uint8_t *data );

#ifdef __cplusplus
}
#endif

#endif