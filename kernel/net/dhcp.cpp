#include <kernel_common.h>
#include <net/udp.h>
#include <net/ipv4.h>
#include <net/ethernet.h>
#include <net/dhcp.h>

void dhcp_start( void ) {
	dhcp_packet packet;

	packet.chaddr[0] = 0x12;
	packet.chaddr[1] = 0x34;
	packet.chaddr[2] = 0x56;
	packet.chaddr[3] = 0x78;
	packet.chaddr[4] = 0x9A;
	packet.chaddr[5] = 0xBC;

	packet.ciaddr = 0;
	packet.yiaddr = 0;
	packet.siaddr = 0;
	packet.giaddr = 0;

	packet.op = DHCP_DISCOVER;
	packet.htype = 1;
	packet.hlen = 6;
	packet.hops = 0;
	packet.xid = htonl(55);
	packet.secs = 0;
	packet.flags = 0;

	packet.magic = htonl( DHCP_MAGIC );

	dhcp_send( 0xFFFFFFFF, &packet );
}

void dhcp_send( uint32_t address, dhcp_packet *packet ) {
	kdebug_peek_at( (uint64_t)packet );

	udp_send( address, DHCP_DESTINATION_PORT, DHCP_SOURCE_PORT, (uint8_t *)packet, sizeof( dhcp_packet ) );
}

void dhcp_process_packet( uint8_t *unprocessed_data ) {
	dhcp_packet *packet = (dhcp_packet *)unprocessed_data;

	switch( packet->op ) {
		case DHCP_OFFER:
			dhcp_handle_offer( packet );
			break;
		case DHCP_ACK:
			dhcp_handle_ack( packet );
			break;
		default:
			debugf( "DHCP unhandled operation: %d\n", packet->op );
	}
}

void dhcp_handle_offer( dhcp_packet *packet ) {
	char ip_string[16];

	debugf( "Got DHCP_OFFER:\n" );
	debugf( "    Assigned IP: %s\n", ip_nota( htonl(packet->yiaddr), ip_string) );
}

void dhcp_handle_ack( dhcp_packet *packet ) {
	debugf( "Got DHCP_ACK:\n" );
}