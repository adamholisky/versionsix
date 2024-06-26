#ifndef VIOS_NET_ARP_INCLUDED
#define VIOS_NET_ARP_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
	uint16_t htype; // Hardware type
	uint16_t ptype; // Protocol type
	uint8_t  hlen; // Hardware address length (Ethernet = 6)
	uint8_t   plen; // Protocol address length (IPv4 = 4)
	uint16_t opcode; // ARP Operation Code
	uint8_t   src_hardware_addr[6]; // Source hardware address - hlen bytes (see above)
	uint8_t   src_protocol_addr[4]; // Source protocol address - plen bytes (see above). If IPv4 can just be a "u32" type.
	uint8_t   dst_hardware_addr[6]; // Destination hardware address - hlen bytes (see above)
	uint8_t   dst_protocol_addr[4]; // Destination protocol address - plen bytes (see above). If IPv4 can just be a "u32" type.
} __attribute__((packed)) arp_packet;

void arp_send( uint8_t *dest_protocol_addr );
void arp_process_packet( arp_packet *packet );

#ifdef __cplusplus
}
#endif
#endif