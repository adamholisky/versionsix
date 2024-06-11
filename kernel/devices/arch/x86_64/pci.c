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
 
	out_port_long(0xCF8, address);

	result = in_port_long(0xCFC);
	
	/* if( result != 0xFFFFFFFF ) {
		debugf( "result: 0x%08X    move: %d    end: 0x%08X\n", result, (8 * (offset & 0x3)), (uint16_t)((result) >> (8 * (offset & 0x2))));
	} */
	
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

	for( int i = 0; i < 0xF; i++ ) {
		address = (uint32_t)((bus32 << 16) | (device32 << 11) | (function32 << 8) | ((uint8_t)(i * 4) & 0xfc) | ((uint32_t)0x80000000));
 
		out_port_long(0xCF8, address);

		result = in_port_long(0xCFC);
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
