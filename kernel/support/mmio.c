#include <kernel_common.h>
#include <page.h>
#include <mmio.h>

void mmio_write32( mmio_config *config, uint64_t *address, uint32_t value ) {
	*((volatile uint32_t *)address) = value;
}

uint32_t mmio_read32( mmio_config *config, uint64_t *address ) {
	debugf( "Addr: %016llX\n", address );
	return *((volatile uint32_t *)address);
}

void mmio_write_command( mmio_config *config, uint32_t reg, uint32_t value ) {
	mmio_write32( config, (uint64_t *)((uint64_t)config->addr + reg), value );
}

uint32_t mmio_read_command( mmio_config *config, uint32_t reg ) {
	debugf( "mmio_config: %016llx\n", config );
	return mmio_read32( config, (uint64_t *)((uint64_t)config->addr + reg) );
}

void mmio_initalize( mmio_config *config, uint64_t *addr ) {
	config->addr = addr;

	// TODO: Fix this, gross, related to e1000 driver
	paging_page_entry *fix_this = paging_get_page_for_virtual_address( (uint64_t)config->addr );
	fix_this->cache_disabled = 1;
	fix_this->write_through = 1;
	
	asm_refresh_cr3();
}