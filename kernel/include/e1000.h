#ifndef VIOS_INTEL8254_INCLUDED
#define VIOS_INTEL8254_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <interrupt.h>
#include <mmio.h>
#include <pci.h>

#define REG_CTRL 0x0000
#define REG_STATUS 0x0008
#define REG_EEPROM 0x0014
#define REG_CTRL_EXT 0x0018
#define REG_IMASK 0x00D0
#define REG_RCTRL 0x0100
#define REG_IMS 0x00d0
#define REG_ICR 0x00C0

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

#define REG_RXADDR 0x5400

#define CTRL_RST 0x4000000
#define CTRL_SLU 0x40

#define RCTL_EN	(1 << 1)
#define RCTL_SBP (1 << 2)
#define RCTL_MPE (1 << 4)
#define RCTL_BAM (1 << 15)
#define RCTL_BSIZE_4096	( (3 << 16) | (1 << 25) ) // Flag for 4096 byte size, plus extended size
#define RCTL_SECRC (1 << 26)

#define TCTL_EN 0x2
#define TCTL_PSP 0x8

#define CMD_EOP (1 << 0)
#define CMD_IFCS (1 << 1)
#define CMD_IC (1 << 2)
#define CMD_RS (1 << 3)
#define CMD_RPS (1 << 4)
#define CMD_VLE (1 << 6)
#define CMD_IDE (1 << 7)

#define ICR_TXDW (1 << 0)
#define ICR_TXQE (1 << 1)
#define ICR_LSC (1 << 2)
#define ICR_RXSEQ (1 << 3)
#define ICR_RXDMT0 (1 << 4)
#define ICR_RXO (1 << 6)
#define ICR_RXT0 (1 << 7)
#define ICR_ACK (1 << 17)
#define ICR_SRPD (1 << 16)

#define E1000_QUEUE_LENGTH 64

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

typedef struct {
	pci_header *pci_info;
	uint16_t io_port;

	e1000_rx_desc *rx_desc_queue;
	uint64_t *rx_data[E1000_QUEUE_LENGTH];
	uint64_t rx_index;

	e1000_tx_desc *tx_desc_queue;
	uint64_t *tx_data[E1000_QUEUE_LENGTH];
	uint64_t tx_index;
	
	uint32_t rx_desc_queue_physical_address;
	uint32_t tx_desc_queue_physical_address;

	irq_handler_func interrupt_handler;

	uint8_t mac_address[8];

	mmio_config *mmio;
	bool has_eeprom;
} e1000_device;

void e1000_initalize( void );
void e1000_interrupt_handler( registers *context );
void e1000_send( uint8_t *data, size_t length );

bool e1000_detect_eeprom( e1000_device *dev );
uint16_t e1000_read_eeprom( e1000_device *dev, uint8_t offset );
uint8_t *e1000_get_mac_address( e1000_device *dev );

void e1000_rx_init( e1000_device *dev );
void e1000_tx_init( e1000_device *dev );

void e1000_send_phase2( e1000_device *dev, uint8_t *data, size_t length );

void e1000_configure( e1000_device *dev, pci_header *pci_header_info );

#ifdef __cplusplus
}
#endif
#endif