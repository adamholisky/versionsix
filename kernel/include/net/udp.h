#ifndef VIOS_NET_UDP_INCLUDED
#define VIOS_NET_UDP_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
	uint16_t source_port;
	uint16_t destination_port;
	uint16_t length;
	uint16_t checksum;
} __attribute__ ((packed)) udp_packet;

void udp_send( uint32_t dest_ip, uint16_t dest_port, uint16_t src_port, uint8_t *data, uint16_t length );
void udp_process_packet( uint8_t *unprocessed_data );

#ifdef __cplusplus
}
#endif
#endif