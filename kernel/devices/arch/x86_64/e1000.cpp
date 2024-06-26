#include <kernel_common.h>
#include <page.h>
#include <kmemory.h>
#include <pci.h>
#include <mmio.h>
#include <e1000.h>
#include <net/ethernet.h>

E1000 *e1000_device;

#define DEBUG_E1000_INITALIZE
extern "C" void e1000_initalize(void) {
	#ifdef DEBUG_E1000_INITALIZE
	log_entry_enter();
	#endif

	interrupts_disable();

	e1000_device = new E1000( pci_get_header_by_device_id(0x100E) );

	uint8_t *MAC = e1000_device->get_mac_address();
	debugf( "MAC Address: %X:%X:%X:%X:%X:%X\n", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5] );

	interrupts_enable();

	#ifdef DEBUG_E1000_INITALIZE
	log_entry_exit();
	#endif
}

extern "C" void e1000_interrupt_handler( registers *context ) {
	dpf( "In handler\n" );

	uint32_t icr = e1000_device->mmio->read_command( REG_ICR );
	dpf( "ICR: 0x%X\n", icr );

	if( icr & 0x03 ) {
		debugf( "Successful send.\n" );
	}

	icr = icr & ~(0x03);

	// we've got a pending packet
	if( icr == 0x80 ) {
		uint32_t head = e1000_device->mmio->read_command( REG_RXDESCHEAD );
		dfv( head );

		// head should not equal the index
		if( head != e1000_device->rx_index ) {
			// process pending packets
			while( e1000_device->rx_desc_queue[e1000_device->rx_index].status & 0x01 ) {
				dpf( "Will process packet at head\n" );
				dfv( e1000_device->rx_desc_queue[e1000_device->rx_index].length );

				// Process packet up the network stack here
				uint64_t *addr_of_data = e1000_device->rx_data[e1000_device->rx_index];
				ethernet_process_packet( addr_of_data );

				// we've proceessed this packet
				e1000_device->rx_desc_queue[e1000_device->rx_index].status = 0;

				// Update the rx index spot
				e1000_device->rx_index++;
				if( e1000_device->rx_index == E1000_QUEUE_LENGTH ) {
					e1000_device->rx_index = 0;
				}

				// Update tail to the new rx index
				e1000_device->mmio->write_command( REG_RXDESCTAIL, e1000_device->rx_index );
				e1000_device->mmio->read_command( REG_STATUS );
			}
		} else {
			dpf( "Head == index, aborting\n" );
		}
	}

	e1000_device->mmio->read_command( REG_ICR );
}

extern "C" void e1000_send( uint8_t *data, size_t length ) {
	e1000_device->send( data, length );
}

uint16_t E1000::read_eeprom( uint8_t offset ) {
	mmio->write_command( REG_EEPROM, 1 | ((uint32_t)(offset) << 8) );

	uint32_t tmp = 0;

	while( !(tmp & (1 << 4)) ) {
		tmp = mmio->read_command( REG_EEPROM );
		__asm__ __volatile__(
			"nop \n\t"
			"nop \n\t"
		);
	}

	return (uint16_t)((tmp >> 16) & 0xFFFF);
}

bool E1000::detect_eeprom( void ) {
	mmio->write_command( REG_EEPROM, 1 );

	for( int i = 0; i < 1000 && !this->has_eeprom; i++ ) {
		uint32_t val = mmio->read_command( REG_EEPROM );

		if( val & 0x10 ) {
			this->has_eeprom = true;
		} else {
			this->has_eeprom = false;
		}
	}

	return this->has_eeprom;
}

uint8_t *E1000::get_mac_address( void ) {
	this->detect_eeprom();

	if( this->has_eeprom ) {
		uint16_t eeprom_data = this->read_eeprom(0);

		this->mac_address[0] = eeprom_data & 0xFF;
		this->mac_address[1]= (eeprom_data >> 8) & 0xFF;

		eeprom_data = this->read_eeprom(1);

		this->mac_address[2] = eeprom_data & 0xFF;
		this->mac_address[3] = (eeprom_data >> 8) & 0xFF;

		eeprom_data = this->read_eeprom(2);

		this->mac_address[4] = eeprom_data & 0xFF;
		this->mac_address[5] = (eeprom_data >> 8) & 0xFF;
	} else {
		// TODO: write this

		uint32_t mac_addr_low  = mmio->read32((uint64_t *)((uint64_t)mmio->addr + REG_RXADDR));
		uint32_t mac_addr_high = mmio->read32((uint64_t *)((uint64_t)mmio->addr + REG_RXADDR + 4));

		debugf( "RXADDR 1: %X\n", mac_addr_high );
		debugf( "RXADDR 2: %x\n", mac_addr_low );

		debugf( "MAC Address NOT from eeprom!\n" );
	}

	return this->mac_address;
}

#define DEBUG_E1000_CONSTRUCTOR
E1000::E1000( pci_header *pci_header_info ) {
	this->pci_info = pci_header_info;

	this->has_eeprom = false;

	for( int i = 0; i < 8; i++ ) {
		mac_address[i] = 0;
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
	this->mmio = new MMIO( (uint64_t *)0x00000000FEB80000 );
	this->mmio->configure_mmu_page();
	this->io_port = (uint16_t)pci_info->bar1;

	uint32_t status = mmio->read_command( REG_STATUS );

	#ifdef DEBUG_E1000_CONSTRUCTOR
	debugf( "io_port addr: 0x%016llx\n", io_port );
	debugf( "mmio_addr: 0x%016llx\n", this->mmio->addr );
	debugf( "status: 0x%X\n", status );
	pci_dump_header( pci_header_info );
	#endif

	// Reset the device
	uint32_t ctrl = mmio->read_command( REG_CTRL );
	ctrl = ctrl | CTRL_RST;
	mmio->write_command( REG_CTRL, ctrl );
	delay( 10000 );

	// Set the link to up
	ctrl = mmio->read_command( REG_CTRL );
	ctrl = ctrl | CTRL_SLU;
	mmio->write_command( REG_CTRL, ctrl );
	delay( 10000 );

	ctrl = mmio->read_command( REG_CTRL );
	debugf( "Control Reg: 0x%X\n", ctrl );

	// TODO: Setup Interrupt Handler
	this->interrupt_handler = e1000_interrupt_handler;
	interrupt_add_irq_handler( pci_info->interrupt_line, interrupt_handler );

	// Initalize transmit and recieve buffers
	debugf( "Initalize rx and tx systems\n" );
	this->rx_init();
	this->tx_init();

	// Enable interrupts
	//mmio->write_command( REG_IMS, (ICR_LSC | ICR_RXO | ICR_RXT0 | ICR_TXQE | ICR_TXDW | ICR_ACK | ICR_RXDMT0 | ICR_SRPD) ); 
	mmio->write_command( REG_IMS, 0x1F6DC );
	mmio->write_command( REG_IMS, 0xFF & ~4 );
	mmio->read_command( REG_ICR );

	/* 	// Send a packet? lol
	debugf( "Sending test packet\n" );
	send( (uint8_t *)"Hello, world?", strlen( "Hello, world?" ) ); */
}

void E1000::rx_init( void ) {
	this->rx_desc_queue = (e1000_rx_desc *)page_allocate_kernel_mmio(2);
	this->rx_desc_queue_physical_address = paging_virtual_to_physical( (uint64_t)this->rx_desc_queue );

	dfv( this->rx_desc_queue_physical_address );

	for( int i = 0; i < E1000_QUEUE_LENGTH; i++ ) {
		this->rx_data[i] = page_allocate_kernel_mmio(1);
		this->rx_desc_queue[i].address = paging_virtual_to_physical( (uint64_t)this->rx_data[i] );
	}

	rx_index = 0;

	mmio->write_command( REG_RXDESCHI, 0 );
	mmio->write_command( REG_RXDESCLO, this->rx_desc_queue_physical_address );
	mmio->write_command( REG_RXDESCLEN, E1000_QUEUE_LENGTH * sizeof(e1000_rx_desc) );
	mmio->write_command( REG_RXDESCHEAD, 0 );
	mmio->write_command( REG_RXDESCTAIL, E1000_QUEUE_LENGTH - 1 );

	mmio->write_command( REG_RCTRL, RCTL_EN | RCTL_SBP | RCTL_MPE | RCTL_BAM | RCTL_BSIZE_4096 | RCTL_SECRC );
}

void E1000::tx_init( void ) {
	this->tx_desc_queue = (e1000_tx_desc *)page_allocate_kernel_mmio(2);
	this->tx_desc_queue_physical_address = paging_virtual_to_physical( (uint64_t)this->tx_desc_queue );

	dfv( this->tx_desc_queue_physical_address );

	for( int i = 0; i < E1000_QUEUE_LENGTH; i++ ) {
		this->tx_data[i]= page_allocate_kernel_mmio(1);
		this->tx_desc_queue[i].address = paging_virtual_to_physical( (uint64_t)this->tx_data[i] );
	}

	tx_index = 0;

	mmio->write_command( REG_TXDESCHI, 0 );
	mmio->write_command( REG_TXDESCLO, this->tx_desc_queue_physical_address );
	mmio->write_command( REG_TXDESCLEN, E1000_QUEUE_LENGTH * sizeof(e1000_tx_desc) );
	mmio->write_command( REG_TXDESCHEAD, 0 );
	mmio->write_command( REG_TXDESCTAIL, 0 );

	mmio->write_command( REG_TCTRL, TCTL_EN | TCTL_PSP );
}

void E1000::send(uint8_t *data, size_t length) {
	uint16_t tail = mmio->read_command( REG_TXDESCTAIL );
	uint16_t head = mmio->read_command( REG_TXDESCHEAD );

	memcpy( tx_data[tx_index], data, length );
	tx_desc_queue[tx_index].length = length;
	tx_desc_queue[tx_index].cmd = CMD_EOP | CMD_IFCS | CMD_RS;
	tx_desc_queue[tx_index].status = 0;

	tx_index++;

	mmio->write_command( REG_TXDESCTAIL, tx_index );
	mmio->read_command( REG_STATUS );
}
