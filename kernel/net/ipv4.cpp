#include <kernel_common.h>
#include <net/udp.h>
#include <net/ipv4.h>
#include <net/ethernet.h>
#include <net/dhcp.h>

void ipv4_send( uint32_t dest, uint8_t protocol, uint8_t *data, uint16_t length ) {
	ipv4_packet packet;
	uint8_t payload[2048];
	uint8_t dest_mac[6];

	packet.version_ihl = ((0x4 << 4) | (0x5 << 0));
	packet.dscp_ecn = 0;
	packet.length = htons(sizeof( ipv4_packet ) + length);
	packet.ident = htons(1);
	packet.flags_fragment = 0;
	packet.ttl = 0x40;
	packet.protocol = IPV4_PROTOCOL_UDP;
	packet.checksum = 0;
	packet.source = htonl(0);
	packet.destination = htonl(dest);

	packet.checksum = htons( calculate_ipv4_checksum(&packet) );

	memset( payload, 0, 2048 );
	memcpy( payload, &packet, sizeof( ipv4_packet ) );
	memcpy( (payload + sizeof( ipv4_packet ) ), data, length );

	// lol don't do this
	if( dest == 0xFFFFFF ) {
		dest_mac[0] = 0xFF;
		dest_mac[1] = 0xFF;
		dest_mac[2] = 0xFF;
		dest_mac[3] = 0xFF;
		dest_mac[4] = 0xFF;
		dest_mac[5] = 0xFF;
	} else if( dest == 0x0A000202 ) {
		dest_mac[0] = 0x52;
		dest_mac[1] = 0x55;
		dest_mac[2] = 0x0A;
		dest_mac[3] = 0x00;
		dest_mac[4] = 0x02;
		dest_mac[5] = 0x02;
	}

	ethernet_send_packet( dest_mac, ETHERNET_TYPE_IPV4, payload, sizeof( ipv4_packet ) + length );
}

void ipv4_process_packet( uint8_t *unprocessed_data ) {
	ipv4_packet *packet = (ipv4_packet *)unprocessed_data;
	uint8_t *data = (uint8_t *)unprocessed_data + sizeof( ipv4_packet );
	char ip_string[16];

	debugf( "Got IPv4 Packet:\n" );
	debugf( "    Protocol: 0x%X (%d)\n", packet->protocol, packet->protocol );
	debugf( "    Source:   %s\n", ip_nota( htonl(packet->source), ip_string) );

	switch( packet->protocol ) {
		case IPV4_PROTOCOL_TCP:
			debugf( "IPV4: TCP not implemented.\n" );
			break;
		case IPV4_PROTOCOL_UDP:
			udp_process_packet( data );
			break;
		default:
			debugf( "IPV4: Uknown protocol: 0x%X (%d)\n", packet->protocol, packet->protocol );
	}
}

uint16_t calculate_ipv4_checksum( ipv4_packet * packet ) {
	uint32_t sum = 0;
	uint16_t * s = (uint16_t *)packet;

	/* TODO: Checksums for options? */
	for (int i = 0; i < 10; ++i) {
		sum += ntohs(s[i]);
	}

	if (sum > 0xFFFF) {
		sum = (sum >> 16) + (sum & 0xFFFF);
	}

	return ~(sum & 0xFFFF) & 0xFFFF;
}

char * ip_nota( uint32_t ip, char * s ) {
	memset( s, 0, 16 );
	snprintf( s, 16, "%d.%d.%d.%d",
			(ip & 0xFF000000) >> 24,
			(ip & 0xFF0000) >> 16,
			(ip & 0xFF00) >> 8,
			(ip & 0xFF) );
	
	return s;
}