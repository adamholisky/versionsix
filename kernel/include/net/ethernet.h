#ifndef VIOS_NET_ETHERNET_INCLUDED
#define VIOS_NET_ETHERNET_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define ETHERNET_TYPE_IPV4 0x0800
#define ETHERNET_TYPE_IPV6 0x86DD
#define ETHERNET_TYPE_ARP 0x0806


typedef struct {
	uint8_t destination[6];
	uint8_t source[6];
	uint16_t type;
} __attribute__((packed)) ethernet_packet;

void ethernet_send_packet( uint8_t *dest, uint16_t type, uint8_t *data, uint32_t length );
void ethernet_process_packet( uint64_t *data );

#ifdef __cplusplus
}
#endif
#endif