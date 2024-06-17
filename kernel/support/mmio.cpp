#include <kernel_common.h>
#include <page.h>
#include <mmio.h>

void MMIO::write32( uint64_t *address, uint32_t value ) {
	*((volatile uint32_t *)address) = value;
}

uint32_t MMIO::read32( uint64_t *address ) {
	return *((volatile uint32_t *)address);
}

void MMIO::write_command( uint32_t reg, uint32_t value ) {
	this->write32( (uint64_t *)((uint64_t)this->addr + reg), value );
}

uint32_t MMIO::read_command( uint32_t reg ) {
	return this->read32( (uint64_t *)((uint64_t)this->addr + reg) );
}

void MMIO::configure_mmu_page( void ) {
    // TODO: Fix this, gross, related to e1000 driver
    paging_page_entry *fix_this = paging_get_page_for_virtual_address( (uint64_t)this->addr );
    fix_this->cache_disabled = 1;
    fix_this->write_through = 1;
    
    asm_refresh_cr3();
}

MMIO::MMIO( uint64_t *address ) {
    this->addr = address;
}