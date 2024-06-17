#ifndef VIOS_INTEL8254_INCLUDED
#define VIOS_INTEL8254_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <mmio.h>
#include <pci.h>

void e1000_initalize( void );

#define REG_CTRL        0x0000
#define REG_STATUS      0x0008
#define REG_EEPROM      0x0014
#define REG_CTRL_EXT    0x0018
#define REG_IMASK       0x00D0
#define REG_RCTRL       0x0100
#define REG_RXDESCLO    0x2800
#define REG_RXDESCHI    0x2804
#define REG_RXDESCLEN   0x2808
#define REG_RXDESCHEAD  0x2810
#define REG_RXDESCTAIL  0x2818
 
#define REG_TCTRL       0x0400
#define REG_TXDESCLO    0x3800
#define REG_TXDESCHI    0x3804
#define REG_TXDESCLEN   0x3808
#define REG_TXDESCHEAD  0x3810
#define REG_TXDESCTAIL  0x3818

#define E1000_REG_RXADDR 0x5400

class E1000 {
    private:
        pci_header *pci_info;
        MMIO *mmio;
        uint16_t io_port;

        uint8_t mac_address[8];
    public:
        bool has_eeprom;

        bool detect_eeprom( void );
        uint16_t read_eeprom( uint8_t offset );
        uint8_t *get_mac_address( void );

        E1000( pci_header *pci_header_info );
};


#ifdef __cplusplus
}
#endif
#endif