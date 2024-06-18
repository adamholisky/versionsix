#ifndef VIOS_INTEL8254_INCLUDED
#define VIOS_INTEL8254_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <interrupt.h>
#include <mmio.h>
#include <pci.h>

void e1000_initalize( void );
void e1000_interrupt_handler( registers *context );

#define REG_CTRL 0x0000
#define REG_STATUS 0x0008
#define REG_EEPROM 0x0014
#define REG_CTRL_EXT 0x0018
#define REG_IMASK 0x00D0
#define REG_RCTRL 0x0100
#define REG_RXDESCLO 0x2800
#define REG_RXDESCHI 0x2804
#define REG_RXDESCLEN 0x2808
#define REG_RXDESCHEAD 0x2810
#define REG_RXDESCTAIL 0x2818
 
#define REG_TCTRL 0x0400
#define REG_TXDESCLO 0x3800
#define REG_TXDESCHI 0x3804
#define REG_TXDESCLEN 0x3808
#define REG_TXDESCHEAD 0x3810
#define REG_TXDESCTAIL 0x3818

#define E1000_REG_RXADDR 0x5400

#define CTRL_RST 0x4000000
#define CTRL_SLU 0x40

#define E100_QUEUE_LENGTH 512

typedef struct {
	uint64_t address;
	uint16_t length;
	uint16_t checksum;
	uint8_t  status;
	uint8_t  errors;
	uint16_t special;
} __attribute__((packed)) e1000_rx_desc;

typedef struct {
	uint64_t address;
	uint16_t length;
	uint8_t  cso;
	uint8_t  cmd;
	uint8_t  status;
	uint8_t  css;
	uint16_t special;
}  __attribute__((packed)) e1000_tx_desc;

class E1000 {
    private:
        pci_header *pci_info;
        MMIO *mmio;
        uint16_t io_port;

        e1000_rx_desc *rx_desc_queue;
        e1000_tx_desc *tx_desc_queue;

        irq_handler_func interrupt_handler;

        uint8_t mac_address[8];
    public:
        bool has_eeprom;

        bool detect_eeprom( void );
        uint16_t read_eeprom( uint8_t offset );
        uint8_t *get_mac_address( void );

        void rx_init( void );
        void tx_init( void );

        E1000( pci_header *pci_header_info );
};


#ifdef __cplusplus
}
#endif
#endif