#include <kernel_common.h>
#include <pci.h>

pci_header	pci_devices[ 15 ];
uint32_t	pci_devices_top;

#undef DEBUG_PCI_INITALIZE
void pci_initalize( void ) {
	uint16_t vendor;
	uint8_t bus;
	uint8_t device;
	pci_header *d;
	int f, i;

	pci_devices_top = 0;

	for( bus = 0; bus < 0xFF; bus++ ) {
		
		for( device = 0; device < 32; device++ ) {
			vendor = 0;

			vendor = pci_config_read( bus, device, 0, 0 );

			if( vendor != 0xFFFF ) {
				pci_read_header( &pci_devices[pci_devices_top], bus, device, 0 );
				pci_devices_top++;

				if( pci_devices[pci_devices_top - 1].device_id == 0x100E ) {
					debugf( "0x100E found: bus = %d, device = %d\n", bus, device );
					/* hack_8254_bus = bus;
					hack_8254_device = device; */
				}

				if( pci_devices[pci_devices_top - 1].device_id == 0x1111 ) {
					debugf( "0x1111 found: bus = %d, device = %d\n", bus, device );
				}

				if( pci_devices[ pci_devices_top - 1 ].header_type == 0x80 ) {
					for( f = 1; f < 8; f++ ) {
						vendor = pci_config_read( bus, device, f, 0 );

						if( vendor != 0xFFFF ) {
							pci_read_header( &pci_devices[pci_devices_top], bus, device, f );
							pci_devices_top++;
						}
					}
				}
			}
		}
	}

	#ifdef DEBUG_PCI_INITALIZE
	debugf( "Num devices: %d", pci_devices_top );
	#endif

	for( i = 0; i < pci_devices_top; i++ ) {
		d = &pci_devices[i];

		if( d->class_code == 0x06 ) continue;

		
		debugf( "[%d] Class: %02X   Subcl: %02X   Prog IF: %02X   Rev: %02X   Ven: %04X   DevID: %04X\n",
			     i, d->class_code, d->subclass, d->prog_if, d->revision_id, d->vendor_id, d->device_id );
		
	}
}

uint16_t pci_config_read( uint8_t bus, uint8_t device, uint8_t function, uint8_t offset ) {
	uint32_t address;
	uint32_t bus32  = (uint32_t)bus;
	uint32_t device32 = (uint32_t)device;
	uint32_t function32 = (uint32_t)function;
	uint32_t result = 0;
 
	address = (uint32_t)((bus32 << 16) | (device32 << 11) | (function32 << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
 
	out_port_long( PCI_CONFIG_ADDRESS, address );

	result = in_port_long( PCI_CONFIG_DATA );
	result = (result) >> (8 * (offset & 0x3));

	return (uint16_t)result;
}

void pci_read_header( pci_header *header, uint8_t bus, uint8_t device, uint8_t function ) {
	uint32_t address;
	uint32_t bus32  = (uint32_t)bus;
	uint32_t device32 = (uint32_t)device;
	uint32_t function32 = (uint32_t)function;
	uint32_t result;
	uint32_t * head = (uint32_t *)header;
	uint8_t offset;

	for( int x = 0; x < 0x40; x = x+4 ) {
		result = pci_read_long( bus, device, function, x );
		*head = result;
		head++;
	}
}

pci_header * pci_get_header_by_device_id( uint32_t _device_id ) {
	pci_header * return_val;
	bool found = false;

	for( int i = 0; i < pci_devices_top; i++ ) {
		return_val = &pci_devices[i];

		if( return_val->device_id == _device_id ) {
			i = pci_devices_top + 1;
			found = true;
		}
	}

	if( !found ) {
		return_val = NULL;
	}

	return return_val;
}

uint32_t pci_read_long( const uint16_t bus, const uint16_t device, const uint16_t func, const uint32_t field ) {
	//out_port_long( PCI_CONFIG_ADDRESS, 0x80000000L | ((uint32_t)bus << 16) |((uint32_t)dev << 11) | ((uint32_t)func << 8) | (reg & ~3) );

	out_port_long( PCI_CONFIG_ADDRESS, 0x80000000 | ((uint8_t)bus << 16) | ((uint8_t)device << 11) | ((uint8_t)func << 8) | ((field) & 0xFC) );

	//return in_port_long( ( PCI_CONFIG_DATA )  >> ((field & 2) * 8) & 0xFFFF );
	return in_port_long( PCI_CONFIG_DATA );
}

uint16_t pci_read_short( const uint16_t bus, const uint16_t device, const uint16_t func, const uint32_t field ) {
	//out_port_long( PCI_CONFIG_ADDRESS, 0x80000000L | ((uint32_t)bus << 16) |((uint32_t)dev << 11) | ((uint32_t)func << 8) | (reg & ~3) );

	out_port_long( PCI_CONFIG_ADDRESS, 0x80000000 | ((uint8_t)bus << 16) | ((uint8_t)device << 11) | ((uint8_t)func << 8) | ((field) & 0xFC) );

	//return in_port_long( ( PCI_CONFIG_DATA )  >> ((field & 2) * 8) & 0xFFFF );
	return in_port_short( PCI_CONFIG_DATA + (field & 2) );
}

/* void pci_write( const uint16_t bus, const uint16_t device, const uint16_t func, const uint32_t field, unsigned data ) {
	out_port_long( PCI_CONFIG_ADDRESS, 0x80000000L | ((uint32_t)bus << 16) |((uint32_t)device << 11) |
	((uint32_t)func << 8) | (field & ~3) );

	out_port_long( PCI_CONFIG_DATA + (field & 3), data );
} */

void pci_write( const uint16_t bus, const uint16_t device, const uint16_t func, const uint32_t field, uint32_t data ) {
	out_port_long( PCI_CONFIG_ADDRESS, 0x80000000 | ((uint8_t)bus << 16) | ((uint8_t)device << 11) | ((uint8_t)func << 8) | ((field) & 0xFC) );

	out_port_long( PCI_CONFIG_DATA, data );
}

void pci_dump_header( pci_header *header ) {
	debugf( "PCI Header Dump:\n" );
	debugf( "    Vendor ID: %X\n", header->vendor_id );
	debugf( "    Device ID: %X\n", header->device_id );
	debugf( "    Revision ID: %X\n", header->revision_id );
	debugf( "    Prog IF: %X\n", header->prog_if );
	debugf( "    Subclass: %X\n", header->subclass );
	debugf( "    Class Code: %X\n", header->class_code );
	debugf( "    Cache Line Size: %X\n", header->cache_line_size );
	debugf( "    Latency Timer: %X\n", header->latency_timer );
	debugf( "    Header Type: %X\n", header->header_type );
	debugf( "    BIST: %X\n", header->bist );
	debugf( "    BAR0: %X\n", header->bar0 );
	debugf( "    BAR1: %X\n", header->bar1 );
	debugf( "    BAR2: %X\n", header->bar2 );
	debugf( "    BAR3: %X\n", header->bar3 );
	debugf( "    BAR4: %X\n", header->bar4 );
	debugf( "    BAR5: %X\n", header->bar5 );
	debugf( "    Subsystem Vendor ID: %X\n", header->subsystem_vendor_id );
	debugf( "    Subsystem ID: %X\n", header->subsystem_id );
	debugf( "    Expansion ROM Address: %X\n", header->expansion_rom_base_address );
	debugf( "    Interrupt Line: %X\n", header->interrupt_line );
	debugf( "    Interrupt Pin: %X\n", header->interrupt_pin );
	debugf( "    Min Grant: %X\n", header->min_grant );
	debugf( "    Max Latency: %X\n", header->max_latency );
}