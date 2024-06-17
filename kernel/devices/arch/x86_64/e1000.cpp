#include <kernel_common.h>
#include <page.h>
#include <pci.h>
#include <e1000.h>

uint64_t *mmio_addr;

uint16_t e1000_read_eeprom( uint8_t offset );
void mmio_write32( uint64_t *addr, uint32_t data );
uint32_t mmio_read32( uint64_t *addr );
bool detect_eeprom( void );
void mmio_write32( uint64_t *address, uint32_t value );
uint32_t mmio_read32( uint64_t *addr );
void write_command( uint32_t reg, uint32_t value );
uint32_t read_command( uint32_t reg );


#define DEBUG_E1000_INITALIZE
extern "C" void e1000_initalize(void) {
	#ifdef DEBUG_E1000_INITALIZE
	log_entry_enter();
	#endif

	pci_header *pci_header_info = pci_get_header_by_device_id(0x100E);

	pci_dump_header( pci_header_info );

	uint64_t *io_port = (uint64_t *)pci_header_info->bar1;
	mmio_addr = (uint64_t *)0x00000000FEB80000;

	// TODO: Fix this once limine gets replaced
	//mmio_addr = page_map( (uint64_t)pci_header_info->bar0, (uint64_t)pci_header_info->bar0 );


	// paging_examine_page_for_address((uint64_t)mmio_addr);
	
	debugf( "io_port addr: 0x%016llx\n", io_port );
	debugf( "mmio_addr: 0x%016llx\n", mmio_addr );

	// 0x7 == Set io space, memory space, bus mastering to active
	uint16_t command_register = pci_read_short( 0, 3, 0, PCI_COMMAND_REGISTER );
	debugf( "data: 0x%04X\n", command_register );
	pci_write( 0, 3, 0, PCI_COMMAND_REGISTER, command_register | 0x7);

	delay( 10000 );

	command_register = pci_read_short( 0, 3, 0, PCI_COMMAND_REGISTER );
	debugf( "data: 0x%04X\n", command_register );


	uint16_t status_register = pci_read_short( 0, 3, 0, PCI_STATUS_REGISTER );
	debugf( "status: 0x%04X\n", status_register );

	// Do we have an eeprom that stores the mac address?
	bool has_eeprom = detect_eeprom();
	debugf( "has_eeprom: %d\n", has_eeprom );

	uint32_t mac_addr_low  = mmio_read32((uint64_t *)((uint64_t)mmio_addr + E1000_REG_RXADDR));
	uint32_t mac_addr_high = mmio_read32((uint64_t *)((uint64_t)mmio_addr + E1000_REG_RXADDR + 4));

	debugf( "RXADDR 1: %X\n", mac_addr_high );
	debugf( "RXADDR 2: %x\n", mac_addr_low );

	uint32_t status = read_command( REG_STATUS );
	debugf( "status: 0x%X\n", status );

	uint16_t eeprom_data = e1000_read_eeprom(0);

	uint16_t mac1 = eeprom_data & 0xFF;
	uint16_t mac2 = (eeprom_data >> 8) & 0xFF;

	eeprom_data = e1000_read_eeprom(1);

	uint16_t mac3 = eeprom_data & 0xFF;
	uint16_t mac4 = (eeprom_data >> 8) & 0xFF;

	eeprom_data = e1000_read_eeprom(2);

	uint16_t mac5 = eeprom_data & 0xFF;
	uint16_t mac6 = (eeprom_data >> 8) & 0xFF;

	debugf( "MAC Address from eeprom: %X:%X:%X:%X:%X:%X\n", mac1, mac2, mac3, mac4, mac5, mac6 );

	#ifdef DEBUG_E1000_INITALIZE
	log_entry_exit();
	#endif
}

uint16_t e1000_read_eeprom( uint8_t offset ) {
	//mmio_write32( (uint64_t *)mmio_addr + 0x0014, 1 | ((uint32_t)(offset) << 8) );
	write_command( REG_EEPROM, 1 | ((uint32_t)(offset) << 8) );

	uint32_t tmp = 0;

	while( !(tmp & (1 << 4)) ) {
		//tmp = mmio_read32( (uint64_t *)mmio_addr + 0x0014 );
		tmp = read_command( REG_EEPROM );
		//debugf( "tmp: %X\n", tmp );
		__asm__ __volatile__(
			"nop \n\t"
			"nop \n\t"
		);
	}

	return (uint16_t)((tmp >> 16) & 0xFFFF);
}

void mmio_write32( uint64_t *address, uint32_t value ) {
	*((volatile uint32_t *)address) = value;
}

uint32_t mmio_read32( uint64_t *addr ) {
	return *((volatile uint32_t *)addr);
}

void write_command( uint32_t reg, uint32_t value ) {
	mmio_write32( (uint64_t *)((uint64_t)mmio_addr + reg), value );
}

uint32_t read_command( uint32_t reg ) {
	return mmio_read32( (uint64_t *)((uint64_t)mmio_addr + reg) );
}

bool detect_eeprom( void ) {
	bool eeprom_exists = false;

	write_command( REG_EEPROM, 1 );

	for( int i = 0; i < 1000 && !eeprom_exists; i++ ) {
		uint32_t val = read_command( REG_EEPROM );

		if( val & 0x10 ) {
			eeprom_exists = true;
		} else {
			eeprom_exists = false;
		}
	}

	return eeprom_exists;
}