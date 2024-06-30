#ifndef VIOS_NET_IPV4_INCLUDED
#define VIOS_NET_IPV4_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define IPV4_PROTOCOL_TCP 6
#define IPV4_PROTOCOL_UDP 17

typedef struct {
	uint8_t  version_ihl;
	uint8_t  dscp_ecn;
	uint16_t length;
	uint16_t ident;
	uint16_t flags_fragment;
	uint8_t  ttl;
	uint8_t  protocol;
	uint16_t checksum;
	uint32_t source;
	uint32_t destination;
	uint8_t  payload[];
} __attribute__((aligned(2))) __attribute__ ((packed)) ipv4_packet;

void ipv4_send( uint32_t dest, uint8_t protocol, uint8_t *data, uint16_t length );
void ipv4_process_packet( uint8_t *unprocessed_data, uint16_t length );
uint16_t calculate_ipv4_checksum( ipv4_packet * packet );
char * ip_nota( uint32_t ip, char * s );
uint32_t ip_to_uint( uint8_t part_a, uint8_t part_b, uint8_t part_c, uint8_t part_d );

#ifdef __cplusplus
}
#endif
#endif