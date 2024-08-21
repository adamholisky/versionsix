#include <kernel_common.h>
#include <page.h>
#include <mmio.h>

void mmio_write32( mmio_config *config, uint64_t *address, uint32_t value ) {
	*((volatile uint32_t *)address) = value;
}

uint32_t mmio_read32( mmio_config *config, uint64_t *address ) {
	return *((volatile uint32_t *)address);
}

void mmio_write_command( mmio_config *config, uint32_t reg, uint32_t value ) {
	mmio_write32( config, (uint64_t *)((uint64_t)config->addr + reg), value );
}

uint32_t mmio_read_command( mmio_config *config, uint32_t reg ) {
	return mmio_read32( config, (uint64_t *)((uint64_t)config->addr + reg) );
}

void mmio_initalize( mmio_config *config, uint64_t *addr ) {
	config->addr = addr + ADDR_IDENTITY_MAP;
}