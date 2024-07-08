#ifndef VIOS_MMIO_INCLUDED
#define VIOS_MMIO_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint64_t *addr;
} mmio_config;

void mmio_initalize( mmio_config *config, uint64_t *addr );
void mmio_write32( mmio_config *config, uint64_t *address, uint32_t value );
uint32_t mmio_read32( mmio_config *config, uint64_t *address );
void mmio_write_command( mmio_config *config, uint32_t reg, uint32_t value );
uint32_t mmio_read_command( mmio_config *config, uint32_t reg );

#ifdef __cplusplus
}
#endif
#endif