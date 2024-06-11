#ifndef VIOS_PCI_INCLUDED
#define VIOS_PCI_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t 	vendor_id;
    uint16_t 	device_id;

    uint16_t 	command;
    uint16_t 	status;
    
	uint8_t 	revision_id;
    uint8_t 	prog_if;
    uint8_t 	subclass;
    uint8_t 	class_code;
    
	uint8_t 	cache_line_size;
    uint8_t 	latency_timer;
    uint8_t 	header_type;
    uint8_t 	bist;
    
	uint32_t 	bar0;
    
	uint32_t 	bar1;
    
	uint32_t 	bar2;
	
	uint32_t	bar3;
	
	uint32_t	bar4;
	
	uint32_t	bar5;
	
	uint32_t	carbus_cis_pointer;
	
	uint16_t	subsystem_vendor_id;
	uint16_t	subsystem_id;
	
	uint32_t	expansion_rom_base_address;
	
	uint8_t		capabilities_pointer;
	uint8_t		reserved1;
	uint16_t	reserved2;
	
	uint32_t	reserved3;
	
	uint8_t		interrupt_line;
	uint8_t		interrupt_pin;
	uint8_t		min_grant;
	uint8_t		max_latency;
} pci_header;

void pci_initalize( void );
uint16_t pci_config_read( uint8_t bus, uint8_t device, uint8_t function, uint8_t offset );
void pci_read_header( pci_header *header, uint8_t bus, uint8_t device, uint8_t function );
pci_header * pci_get_header_by_device_id( uint32_t _device_id );

#ifdef __cplusplus
}
#endif
#endif