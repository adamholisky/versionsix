#include <kernel_common.h>
#include <page.h>
#include <kmemory.h>
#include <pci.h>
#include <mmio.h>
#include <e1000.h>
#include <net/ethernet.h>

e1000_device *main_e1000;

#define DEBUG_E1000_INITALIZE
void e1000_initalize(void) {
	#ifdef DEBUG_E1000_INITALIZE
	log_entry_enter();
	#endif

	interrupts_disable();

	main_e1000 = kmalloc( sizeof(e1000_device) );
	
	e1000_configure( main_e1000, pci_get_header_by_device_id(0x100E) );

	uint8_t *MAC = e1000_get_mac_address( main_e1000 );
	debugf( "MAC Address: %X:%X:%X:%X:%X:%X\n", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5] );

	interrupts_enable();

	#ifdef DEBUG_E1000_INITALIZE
	log_entry_exit();
	#endif
}

void e1000_interrupt_handler( registers **context ) {
	//dpf( "In handler\n" );

	uint32_t icr = mmio_read_command( main_e1000->mmio, REG_ICR );
	//dpf( "ICR: 0x%X\n", icr );

	if( icr & 0x03 ) {
		//debugf( "Successful send.\n" );
	}

	icr = icr & ~(0x03);

	// we've got a pending packet
	if( icr == 0x80 ) {
		uint32_t head = mmio_read_command( main_e1000->mmio, REG_RXDESCHEAD );
		//dfv( head );

		if( head == main_e1000->rx_index ) {
			mmio_read_command( main_e1000->mmio, REG_RXDESCHEAD );
		}

		// head should not equal the index
		if( head != main_e1000->rx_index ) {
			int i = 1;
			
			// process pending packets
			while( main_e1000->rx_desc_queue[main_e1000->rx_index].status & 0x01 ) {
				// dpf( "Will process packet at head\n" );
				//dfv( e1000_device->rx_desc_queue[e1000_device->rx_index].length );

				// Process packet up the network stack here
				uint64_t *addr_of_data = main_e1000->rx_data[main_e1000->rx_index];
				ethernet_process_packet( addr_of_data, main_e1000->rx_desc_queue[main_e1000->rx_index].length );

				// we've proceessed this packet
				main_e1000->rx_desc_queue[main_e1000->rx_index].status = 0;

				// Update the rx index spot
				main_e1000->rx_index++;
				if( main_e1000->rx_index == E1000_QUEUE_LENGTH ) {
					main_e1000->rx_index = 0;
				}

				if( main_e1000->rx_index == head ) {
					head = mmio_read_command( main_e1000->mmio, REG_RXDESCHEAD );
				
					if( head == main_e1000->rx_index ) {
						break;
					}
				}

				// Update tail to the new rx index
				mmio_write_command( main_e1000->mmio, REG_RXDESCTAIL, main_e1000->rx_index );
				mmio_read_command( main_e1000->mmio, REG_STATUS );
			}
		} else {
			dpf( "Head == index, aborting\n" );
		}
	}
}

void e1000_send( uint8_t *data, size_t length ) {
	e1000_send_phase2( main_e1000, data, length );
}

uint16_t e1000_read_eeprom( e1000_device *dev, uint8_t offset ) {
	mmio_write_command( dev->mmio, REG_EEPROM, 1 | ((uint32_t)(offset) << 8) );

	uint32_t tmp = 0;

	while( !(tmp & (1 << 4)) ) {
		tmp = mmio_read_command( dev->mmio, REG_EEPROM );
		__asm__ __volatile__(
			"nop \n\t"
			"nop \n\t"
		);
	}

	return (uint16_t)((tmp >> 16) & 0xFFFF);
}

bool e1000_detect_eeprom( e1000_device *dev ) {
	mmio_write_command( dev->mmio, REG_EEPROM, 1 );

	for( int i = 0; i < 1000 && !dev->has_eeprom; i++ ) {
		uint32_t val = mmio_read_command( dev->mmio, REG_EEPROM );

		if( val & 0x10 ) {
			dev->has_eeprom = true;
		} else {
			dev->has_eeprom = false;
		}
	}

	return dev->has_eeprom;
}

uint8_t *e1000_get_mac_address( e1000_device *dev ) {
	e1000_detect_eeprom( dev );

	if( dev->has_eeprom ) {
		uint16_t eeprom_data = e1000_read_eeprom(dev, 0);

		dev->mac_address[0] = eeprom_data & 0xFF;
		dev->mac_address[1]= (eeprom_data >> 8) & 0xFF;

		eeprom_data = e1000_read_eeprom(dev, 1);

		dev->mac_address[2] = eeprom_data & 0xFF;
		dev->mac_address[3] = (eeprom_data >> 8) & 0xFF;

		eeprom_data = e1000_read_eeprom(dev, 2);

		dev->mac_address[4] = eeprom_data & 0xFF;
		dev->mac_address[5] = (eeprom_data >> 8) & 0xFF;
	} else {
		// TODO: write this

		uint32_t mac_addr_low  = mmio_read32(dev->mmio, (uint64_t *)((uint64_t)dev->mmio->addr + REG_RXADDR));
		uint32_t mac_addr_high = mmio_read32(dev->mmio, (uint64_t *)((uint64_t)dev->mmio->addr + REG_RXADDR + 4));

		debugf( "RXADDR 1: %X\n", mac_addr_high );
		debugf( "RXADDR 2: %x\n", mac_addr_low );

		debugf( "MAC Address NOT from eeprom!\n" );
	}

	return dev->mac_address;
}

#define DEBUG_E1000_CONSTRUCTOR
void e1000_configure( e1000_device *dev, pci_header *pci_header_info ) {
	dev->pci_info = pci_header_info;

	dev->has_eeprom = false;

	for( int i = 0; i < 8; i++ ) {
		dev->mac_address[i] = 0;
	}

	// Enable bus mastering
	// 0x7 == Set io space, memory space, bus mastering to active
	uint16_t command_register = pci_read_short( 0, 3, 0, PCI_COMMAND_REGISTER );
	//debugf( "data: 0x%04X\n", command_register );
	pci_write( 0, 3, 0, PCI_COMMAND_REGISTER, command_register | 0x7);

	delay( 10000 );

	command_register = pci_read_short( 0, 3, 0, PCI_COMMAND_REGISTER );
	//debugf( "data: 0x%04X\n", command_register );

	// TODO: Fix this once limine gets replaced
	//uint64_t *mmio_addr = page_map( (uint64_t)pci_header_info->bar0, (uint64_t)pci_header_info->bar0 );
	dev->mmio = kmalloc( sizeof(mmio_config) );
	mmio_initalize( dev->mmio, (uint64_t *)0x00000000FEB80000 );
	dev->io_port = (uint16_t)dev->pci_info->bar1;

	uint32_t status = mmio_read_command( dev->mmio, REG_STATUS );

	#ifdef DEBUG_E1000_CONSTRUCTOR
	debugf( "io_port addr: 0x%016llx\n", dev->io_port );
	debugf( "mmio_addr: 0x%016llx\n", dev->mmio->addr );
	debugf( "status: 0x%X\n", status );
	pci_dump_header( pci_header_info );
	#endif

	// Reset the device
	uint32_t ctrl = mmio_read_command( dev->mmio, REG_CTRL );
	ctrl = ctrl | CTRL_RST;
	mmio_write_command( dev->mmio, REG_CTRL, ctrl );
	delay( 10000 );

	// Set the link to up
	ctrl = mmio_read_command( dev->mmio, REG_CTRL );
	ctrl = ctrl | CTRL_SLU;
	mmio_write_command( dev->mmio, REG_CTRL, ctrl );
	delay( 10000 );

	ctrl = mmio_read_command( dev->mmio, REG_CTRL );
	debugf( "Control Reg: 0x%X\n", ctrl );

	// TODO: Setup Interrupt Handler
	dev->interrupt_handler = e1000_interrupt_handler;
	interrupt_add_irq_handler( dev->pci_info->interrupt_line, dev->interrupt_handler );

	// Initalize transmit and recieve buffers
	debugf( "Initalize rx and tx systems\n" );
	e1000_rx_init( dev );
	e1000_tx_init( dev );

	// Enable interrupts
	//mmio->write_command( REG_IMS, (ICR_LSC | ICR_RXO | ICR_RXT0 | ICR_TXQE | ICR_TXDW | ICR_ACK | ICR_RXDMT0 | ICR_SRPD) ); 
	mmio_write_command( dev->mmio, REG_IMS, 0x1F6DC );
	mmio_write_command( dev->mmio, REG_IMS, 0xFF & ~4 );
	mmio_read_command( dev->mmio, REG_ICR );

	/* 	// Send a packet? lol
	debugf( "Sending test packet\n" );
	send( (uint8_t *)"Hello, world?", strlen( "Hello, world?" ) ); */
}

void e1000_rx_init( e1000_device *dev ) {
	dev->rx_desc_queue = (e1000_rx_desc *)page_allocate_kernel_mmio(2);
	dev->rx_desc_queue_physical_address = paging_virtual_to_physical( (uint64_t)dev->rx_desc_queue );

	dfv( dev->rx_desc_queue_physical_address );

	for( int i = 0; i < E1000_QUEUE_LENGTH; i++ ) {
		dev->rx_data[i] = page_allocate_kernel_mmio(1);
		dev->rx_desc_queue[i].address = paging_virtual_to_physical( (uint64_t)dev->rx_data[i] );
	}

	dev->rx_index = 0;

	mmio_write_command( dev->mmio, REG_RXDESCHI, 0 );
	mmio_write_command( dev->mmio, REG_RXDESCLO, dev->rx_desc_queue_physical_address );
	mmio_write_command( dev->mmio, REG_RXDESCLEN, E1000_QUEUE_LENGTH * sizeof(e1000_rx_desc) );
	mmio_write_command( dev->mmio, REG_RXDESCHEAD, 0 );
	mmio_write_command( dev->mmio, REG_RXDESCTAIL, E1000_QUEUE_LENGTH - 1 );

	mmio_write_command( dev->mmio, REG_RCTRL, RCTL_EN | RCTL_SBP | RCTL_MPE | RCTL_BAM | RCTL_BSIZE_4096 | RCTL_SECRC );
}

void e1000_tx_init( e1000_device *dev ) {
	dev->tx_desc_queue = (e1000_tx_desc *)page_allocate_kernel_mmio(2);
	dev->tx_desc_queue_physical_address = paging_virtual_to_physical( (uint64_t)dev->tx_desc_queue );

	dfv( dev->tx_desc_queue_physical_address );

	for( int i = 0; i < E1000_QUEUE_LENGTH; i++ ) {
		dev->tx_data[i]= page_allocate_kernel_mmio(1);
		dev->tx_desc_queue[i].address = paging_virtual_to_physical( (uint64_t)dev->tx_data[i] );
	}

	dev->tx_index = 0;

	mmio_write_command( dev->mmio, REG_TXDESCHI, 0 );
	mmio_write_command( dev->mmio, REG_TXDESCLO, dev->tx_desc_queue_physical_address );
	mmio_write_command( dev->mmio, REG_TXDESCLEN, E1000_QUEUE_LENGTH * sizeof(e1000_tx_desc) );
	mmio_write_command( dev->mmio, REG_TXDESCHEAD, 0 );
	mmio_write_command( dev->mmio, REG_TXDESCTAIL, 0 );

	mmio_write_command( dev->mmio, REG_TCTRL, TCTL_EN | TCTL_PSP );
}

void e1000_send_phase2( e1000_device *dev, uint8_t *data, size_t length) {
	uint16_t tail = mmio_read_command( dev->mmio, REG_TXDESCTAIL );
	uint16_t head = mmio_read_command( dev->mmio, REG_TXDESCHEAD );

	memcpy( dev->tx_data[dev->tx_index], data, length );
	dev->tx_desc_queue[dev->tx_index].length = length;
	dev->tx_desc_queue[dev->tx_index].cmd = CMD_EOP | CMD_IFCS | CMD_RS;
	dev->tx_desc_queue[dev->tx_index].status = 0;

	dev->tx_index++;

	mmio_write_command( dev->mmio, REG_TXDESCTAIL, dev->tx_index );
	mmio_read_command( dev->mmio, REG_STATUS );
}
