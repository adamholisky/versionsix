#include <kernel_common.h>
#include <pci.h>
#include <mmio.h>
#include <ahci.h>
#include <page.h>

extern void asm_refresh_cr3( void );

uint16_t *global_buffer;
uint64_t global_buffer_phys;

uint32_t *global_port_page;
uint64_t global_port_page_phys;

uint32_t *global_port_clb;
uint32_t *global_port_fis;
HBA_CMD_HEADER *global_cmd_header;

HBA_MEM * abar;
mmio_config ahci_mmio;
int num_cmd_slots;

#define KDEBUG_AHCI_INIT
void ahci_initalize( void ) {
	log_entry_enter();

	pci_header *pci_ahci_drive = pci_get_header_by_device_id( 0x2922 );
	pci_dump_header( pci_ahci_drive );

	mmio_initalize( &ahci_mmio, pci_ahci_drive->bar5 );
	abar = (HBA_MEM *)pci_ahci_drive->bar5;

	// TODO: use the results to handle whatever drive is attached, for now we hard code
	ahci_probe_port( abar );

	// Get the number of command slots
	num_cmd_slots = (abar->cap & 0x0f00) >> 8;

	// port_page gets the port control 
	global_port_page = page_allocate_kernel_mmio( 1 );
	global_port_page_phys = paging_virtual_to_physical( global_port_page );
	memset( global_port_page, 0, PAGE_SIZE );

	#ifdef KDEBUG_AHCI_INIT
	debugf( "global_port_page virt: %llX\n", global_port_page );
	debugf( "global_port_page Phys: %llX\n", global_port_page_phys );
	#endif

	// TODO: Remove hard coding for "1" here
	ahci_port_rebase( &abar->ports[1], 1, global_port_page, global_port_page_phys );

	global_buffer = page_allocate_kernel_mmio(2);
	global_buffer_phys = paging_virtual_to_physical( global_buffer );

	#ifdef KDEBUG_AHCI_INIT
	debugf( "global_buffer virt: %llX\n", global_buffer );
	debugf( "global_buffer phys: %llX\n", global_buffer_phys );
	#endif

	bool read_result = read_ahci( &abar->ports[1], 0,0,16, (uint16_t *)global_buffer_phys );

	#ifdef KDEBUG_AHCI_INIT
	debugf( "Read_result: %d\n", read_result );
	#endif

	debugf( "Buffer: \n" );
	
	int z = 0;
	for( int b = 0; b < 50; b++ ) {
		if (z == 0) {
			debugf_raw( "\n%04X    ", b * 2 );
		}
		
		debugf_raw( "%02X %02X  ", ( 0x00FF ) & *(global_buffer + b), ((0xFF00) & *(global_buffer + b))>>8 );

		z++;

		if( z == 0x8 ) {
			z = 0;
		}
	} 
 	
	log_entry_exit();
}

/**
 * @brief Probe the AHCI memory for what ports have a device attached, including what type it is
 * 
 * @param abar Pointer to the AHCI ABAR
 */
void ahci_probe_port( HBA_MEM *abar ) {
	uint32_t pi = abar->pi; // Ports implemented
	
	// Max of 32 ports
	for( uint8_t i; i < 32; i++ ) {
		// If the bit is set, then the port is available for use
		if( pi & 1 ) {
			uint8_t dt = ahci_check_type( &abar->ports[i] );

			if( dt == AHCI_DEV_SATA ) {
				debugf("SATA drive found at port %d\n", i);
			}
			else if ( dt == AHCI_DEV_SATAPI ) {
				debugf("SATAPI drive found at port %d\n", i);
			}
			else if ( dt == AHCI_DEV_SEMB )	{
				debugf("SEMB drive found at port %d\n", i);
			}
			else if ( dt == AHCI_DEV_PM ) {
				debugf("PM drive found at port %d\n", i);
			}
			else {
				debugf("No drive found at port %d\n", i);
			}
		}

		// PI gets itself with a new bit in position 1 for testing again
		pi >>= 1;
	}
}
 
/**
 * @brief Returns the type of drive attached to the supplied port
 * 
 * @param port Pointer to the HBA_PORT to query
 * @return uint8_t Type of drive attached, or NULL is nothing
 */
uint8_t ahci_check_type( HBA_PORT *port ) {
	uint32_t ssts = port->ssts; // SATA status
 
	uint8_t ipm = (ssts >> 8) & 0x0F; // Interface power management, bits 8, 9, 10, 11
	uint8_t det = ssts & 0x0F; // Device detection, bits 0, 1, 2, 3
	
	// Ensure the drive is present and active
	if( det != HBA_PORT_DET_PRESENT ) {
		return AHCI_DEV_NULL;
	}
	
	if (ipm != HBA_PORT_IPM_ACTIVE) {
		return AHCI_DEV_NULL;
	}
	
	// Return what's in the signature
	switch( port->sig )	{
		case SATA_SIG_ATAPI:
			return AHCI_DEV_SATAPI;
		case SATA_SIG_SEMB:
			return AHCI_DEV_SEMB;
		case SATA_SIG_PM:
			return AHCI_DEV_PM;
		default:
			return AHCI_DEV_SATA;
	}
}

/**
 * @brief Rebase the given port to usable memory that we've set up
 * 
 * @param port pointer to the HBA_PORT
 * @param portno port number (device number) 
 * @param new_page_virt_addr virtaul address of the new port's memory
 * @param new_base_phys_addr physical address of the new port's memory
 */
void ahci_port_rebase( HBA_PORT *port, int portno, uint64_t new_page_virt_addr, uint32_t new_base_phys_addr ) {
	ahci_stop_cmd( port );
 
	// Setup the command list -- this assumes physical addr is within 32-bit memory space
	// Overall size of the command list is 1024 bytes (0x400)
	port->clb = new_base_phys_addr;
	port->clbu = 0;
	global_port_clb = new_page_virt_addr;
 
	// Setup the fis space -- this assumes physical addr is within 32-bit memory space
	// Overall size of the FIS is 256 bytes (0x100)
	port->fb = new_base_phys_addr + 0x400;
	port->fbu = 0;
	global_port_fis = new_page_virt_addr + 0x400;
 
	// Set up the command table
	// Offset is 0x500 (clb + cmd_table_start + fis size) + 8 PRDTS from previous iteration
	// command tables start after 32 cmd_headers == 0x100 (8 bytes * 32)
	// 256 bytes (0x100) per command table
	global_cmd_header = (HBA_CMD_HEADER *)(global_port_clb);
	for( int i = 0; i < 32; i++ ) {
		global_cmd_header[i].prdtl = 8;
		global_cmd_header[i].ctba = new_base_phys_addr + 0x500 + 0x100 + (i * 0x100);
		global_cmd_header[i].ctbau = 0;
	}
 
	ahci_start_cmd( port );
}
 
/**
 * @brief Allow commands for the given port
 * 
 * @param port pointer to the port
 */
void ahci_start_cmd( HBA_PORT *port ) {
	// Wait until the command register is cleared
	while( port->cmd & HBA_PxCMD_CR ){
		;
	}

	port->cmd |= HBA_PxCMD_FRE; // Enable port receiving 
	port->cmd |= HBA_PxCMD_ST; // Set the start bit, allowing processing of the command list
}
 
/**
 * @brief Stop allowing commands for the given port
 * 
 * @param port pointer to the port
 */
void ahci_stop_cmd( HBA_PORT *port ) {
	port->cmd &= ~HBA_PxCMD_ST; // Disable the start bit, disallowing processing of the command list
	port->cmd &= ~HBA_PxCMD_FRE; // Diable port receiving
 
	// Wait until FR (FIS receiving happening) and CR (command list running) are cleared
	while( 1 ) {
		if (port->cmd & HBA_PxCMD_FR) {
			continue;
		}
			
		if (port->cmd & HBA_PxCMD_CR) {
			continue;
		}
			
		break;
	}
 
}

#undef KDEBUG_READ_AHCI
bool read_ahci( HBA_PORT *port, uint32_t startl, uint32_t starth, uint32_t count, uint16_t *buf ) {
	port->is = (uint32_t) -1;		// Clear pending interrupt bits
	int spin = 0; // Spin lock timeout counter
	int slot = find_cmdslot(port);
	if (slot == -1)
		return false;

	#ifdef KDEBUG_READ_AHCI
	debugf( "clb virt: %llx\n", global_port_clb );
	debugf( "clb phys: %X\n", port->clb );
	debugf( "cmd slot: %d\n", slot );
	#endif

	HBA_CMD_HEADER *cmd_header = global_cmd_header;
	
	#ifdef KDEBUG_READ_AHCI
	debugf( "cmd_header: %X\n", cmd_header );
	#endif

	cmd_header += slot;
	cmd_header->cfl = sizeof(FIS_REG_H2D)/sizeof(uint32_t);	// Command FIS size
	cmd_header->w = 0;		// Read from device
	cmd_header->prdtl = (uint16_t)((count-1)>>4) + 1;	// PRDT entries count
	
	// the command table's physical addr is located at cmd_header->ctba 
	// table's offset is ctba - global_port_page_phys (ctba starts at the top of the page)
	// virtual address is global_clb + offset
	uint32_t cmd_table_offset = cmd_header->ctba - global_port_page_phys;
	HBA_CMD_TBL *cmd_tbl = (HBA_CMD_TBL*)((uint8_t *)global_port_clb + cmd_table_offset);

	#ifdef KDEBUG_READ_AHCI
	debugf( "cmd_header->cfl: %llX\n", cmd_header->cfl );
	debugf( "cmd_header->ctba: %llX\n", cmd_header->ctba );
	debugf( "cmd_header->prdtl: %d\n", cmd_header->prdtl );
	debugf( "cmd_table_offset: %llX\n", cmd_table_offset );
	debugf( "cmd_table: %X\n", cmd_tbl );
	#endif

	memset(cmd_tbl, 0, sizeof(HBA_CMD_TBL) + (cmd_header->prdtl-1)*sizeof(HBA_PRDT_ENTRY));

	// 8K bytes (16 sectors) per PRDT
	int i;
	for (i=0; i<cmd_header->prdtl-1; i++)
	{
		cmd_tbl->prdt_entry[i].dba = (uint32_t)global_buffer_phys;
		cmd_tbl->prdt_entry[i].dbc = 8*1024-1;	// 8K bytes (this value should always be set to 1 less than the actual value)
		cmd_tbl->prdt_entry[i].i = 1;
		buf += 4*1024;	// 4K words
		count -= 16;	// 16 sectors
	}
	// Last entry
	cmd_tbl->prdt_entry[i].dba = (uint32_t)global_buffer_phys;
	cmd_tbl->prdt_entry[i].dbc = (count<<9)-1;	// 512 bytes per sector
	cmd_tbl->prdt_entry[i].i = 1;
 
	//debugf( "cmd_tbl->cfis: %llX\n", cmd_tbl->cfis );
	// Setup command
	FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmd_tbl->cfis);
	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1;	// Command
	cmdfis->command = ATA_CMD_READ_DMA_EX;
 
	cmdfis->lba0 = (uint8_t)startl;
	cmdfis->lba1 = (uint8_t)(startl>>8);
	cmdfis->lba2 = (uint8_t)(startl>>16);
	cmdfis->device = 1<<6;	// LBA mode
 
	cmdfis->lba3 = (uint8_t)(startl>>24);
	cmdfis->lba4 = (uint8_t)starth;
	cmdfis->lba5 = (uint8_t)(starth>>8);
 
	cmdfis->countl = count & 0xFF;
	cmdfis->counth = (count >> 8) & 0xFF;

	
 
	// The below loop waits until the port is no longer busy before issuing a new command
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		debugf("Port is hung\n");
		return false;
	}
 
	port->ci = 1<<slot;	// Issue command
 
	// Wait for completion
	while (1)
	{
		//debugf( "#" );
		// In some longer duration reads, it may be helpful to spin on the DPS bit 
		// in the PxIS port field as well (1 << 5)
		if ((port->ci & (1<<slot)) == 0) 
			break;
		if (port->is & HBA_PxIS_TFES)	// Task file error
		{
			debugf("Read disk error\n");
			return false;
		}
	}

	// Check again
	if (port->is & HBA_PxIS_TFES)
	{
		debugf("Read disk error\n");
		return false;
	}

	return true;
}

bool ahci_read_sector( uint32_t sector, uint32_t *buffer ) {
	bool read_result = false;

	read_result = read_ahci( &abar->ports[1], sector, 0, 1, (uint16_t *)global_buffer_phys );

	if( !read_result ) {
		debugf( "Read failed. sector = %X\n", sector );
		return false;
	}

	memcpy( buffer, global_buffer, 512 );

	return true;
}

#undef KDEBUG_AHCI_READ_AT_BYTE_OFFSET
bool ahci_read_at_byte_offset( uint32_t offset, uint32_t size, uint8_t *buffer ) {
	debugf( "offset = 0x%08X, size = 0x%08X, buffer=0x%llx\n", offset, size, buffer );

	bool read_result = false;
	uint32_t sector = 0;
	uint32_t count = 0;
	uint32_t internal_offset = 0;

	sector = offset / 512;
	internal_offset = offset - (sector * 512);
	count = (size / 512) + 2;

	#ifdef KDEBUG_AHCI_READ_AT_BYTE_OFFSET
	debugf( "read: offset -- %X, size %X\n", offset, size );
	debugf( "read at offset -- sector = %X, count = %X, internal_offset = %X\n", sector, count, internal_offset );

	memset( global_buffer, 0xDD, 2 * 1024 * 1024 );
	#endif

	read_result = read_ahci( &abar->ports[1], sector, 0, count, (uint16_t *)global_buffer_phys );

	if( !read_result ) {
		debugf( "Read failed. sector = %X\n", sector );
		return false;
	}
	
	memcpy( buffer, (uint8_t *)global_buffer + internal_offset, size );

	#ifdef KDEBUG_AHCI_READ_AT_BYTE_OFFSET
	int z = 0;
	for( int b = 0; b < (size / 2); b++ ) {
		if (z == 0) {
			//debugf( "\n%04X    ", b * 2 );
		}
		

		//debugf( "%02X %02X  ", ( 0x00FF ) & *(buffer + b), ((0xFF00) & *(buffer + b))>>8 );

		z++;

		if( z == 0x8 ) {
			z = 0;
		}
	}

	debugf( "\n" );

	//kdebug_peek_at_n(global_buffer, 300 );

	//kdebug_peek_at_n(buffer, 300 );
	#endif

	

	return true;
}

bool ahci_read_at_byte_offset_512_chunks( uint32_t offset, uint32_t size, uint8_t *buffer ) {
	bool read_result = false;
	uint32_t sector = 0;
	uint32_t count = 0;
	uint32_t internal_offset = 0;

	sector = offset / 512;
	internal_offset = offset - (sector * 512);
	count = (size / 512) + 2;

	#ifdef KDEBUG_AHCI_READ_AT_BYTE_OFFSET
	debugf( "read: offset -- %X, size %X\n", offset, size );
	debugf( "read at offset -- sector = %X, count = %X, internal_offset = %X\n", sector, count, internal_offset );

	memset( global_buffer, 0xDD, 2 * 1024 * 1024 );
	#endif

	for( int i = 0; i < count; i++ ) {
		read_result = read_ahci( &abar->ports[1], sector + i, 0, 1, (uint16_t *)(global_buffer + (i * 512)) );

		if( !read_result ) {
			debugf( "Read failed. sector = %X\n", sector );
			return false;
		}
	}

	memcpy( buffer, (uint8_t *)global_buffer + internal_offset, size );

	#ifdef KDEBUG_AHCI_READ_AT_BYTE_OFFSET
	int z = 0;
	for( int b = 0; b < (size / 2); b++ ) {
		if (z == 0) {
			//debugf( "\n%04X    ", b * 2 );
		}
		

		//debugf( "%02X %02X  ", ( 0x00FF ) & *(buffer + b), ((0xFF00) & *(buffer + b))>>8 );

		z++;

		if( z == 0x8 ) {
			z = 0;
		}
	}

	debugf( "\n" );

	//kdebug_peek_at_n(global_buffer, 300 );

	//kdebug_peek_at_n(buffer, 300 );
	#endif

	

	return true;
}






 
// Find a free command list slot
int find_cmdslot(HBA_PORT *port)
{
	// If not set in SACT and CI, the slot is free
	uint32_t slots = (port->sact | port->ci);
	for (int i=0; i< num_cmd_slots; i++)
	{
		if ((slots&1) == 0)
			return i;
		slots >>= 1;
	}
	debugf("Cannot find free command list entry\n");
	return -1;
}