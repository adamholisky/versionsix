#include <kernel_common.h>
#include <net/udp.h>
#include <net/ipv4.h>
#include <net/ethernet.h>
#include <net/dhcp.h>
#include <net/network.h>

extern net_info networking_info;
bool send_dhcp_request;
dhcp_packet dhcp_request_packet;

void dhcp_start( void ) {
	dhcp_payload payload;

	memset( &payload, 0, sizeof( dhcp_payload ) );

	payload.header.chaddr[0] = 0x12;
	payload.header.chaddr[1] = 0x34;
	payload.header.chaddr[2] = 0x56;
	payload.header.chaddr[3] = 0x78;
	payload.header.chaddr[4] = 0x9A;
	payload.header.chaddr[5] = 0xBC;

	payload.header.ciaddr = 0;
	payload.header.yiaddr = 0;
	payload.header.siaddr = 0;
	payload.header.giaddr = 0;

	payload.header.op = DHCP_DISCOVER;
	payload.header.htype = 1;
	payload.header.hlen = 6;
	payload.header.hops = 0;
	payload.header.xid = htonl(55);
	payload.header.secs = 0;
	payload.header.flags = 0;

	payload.header.magic = htonl( DHCP_MAGIC );

	payload.options[0] = DHCP_OPTION_TYPE;
	payload.options[1] = 1;
	payload.options[2] = DHCP_DISCOVER;

	payload.options[3] = DHCP_OPTION_PARAM_REQUEST_LIST;
	payload.options[4] = 2;
	payload.options[5] = 3;
	payload.options[6] = 6;
	payload.options[7] = 255;

	send_dhcp_request = false;
	udp_send( DHCP_ADDR_BROADCAST, DHCP_DESTINATION_PORT, DHCP_SOURCE_PORT, (uint8_t *)&payload, sizeof( dhcp_packet ) + 8 );

	while( send_dhcp_request == false ) {
		// do nothing, wait
	}

	dhcp_handle_offer_stage_2();
}

void dhcp_process_packet( uint8_t *unprocessed_data ) {
	dhcp_packet *packet = (dhcp_packet *)unprocessed_data;
	uint8_t *options = (uint8_t *)unprocessed_data + sizeof( dhcp_packet );

	if( packet->op == DHCP_BOOT_REPLY ) {
		switch( options[2] ) {
			case DHCP_OFFER:
				dhcp_handle_offer( packet );
				break;
			case DHCP_ACK:
				dhcp_handle_ack( packet, options );
				break;
			default:
				debugf( "DHCP unhandled operation in boot_reply: %d\n", options[2] );
		}
	} else {
		debugf( "DHCP unhandled operation: %d\n", packet->op );
	}
	
}

void dhcp_handle_offer( dhcp_packet *packet ) {
	memcpy( &dhcp_request_packet, packet, sizeof( dhcp_packet ) );
	send_dhcp_request = true;
}

void dhcp_handle_offer_stage_2( void ) {
	char ip_string[16];
	dhcp_payload request_payload;

	memset( &request_payload, 0, sizeof( dhcp_payload ) );

	request_payload.header.chaddr[0] = 0x12;
	request_payload.header.chaddr[1] = 0x34;
	request_payload.header.chaddr[2] = 0x56;
	request_payload.header.chaddr[3] = 0x78;
	request_payload.header.chaddr[4] = 0x9A;
	request_payload.header.chaddr[5] = 0xBC;

	request_payload.header.ciaddr = 0;
	request_payload.header.yiaddr = 0;
	request_payload.header.siaddr = dhcp_request_packet.siaddr;
	request_payload.header.giaddr = 0;

	request_payload.header.op = DHCP_DISCOVER;
	request_payload.header.htype = 1;
	request_payload.header.hlen = 6;
	request_payload.header.hops = 0;
	request_payload.header.xid = htonl(55);
	request_payload.header.secs = 0;
	request_payload.header.flags = 0;

	request_payload.header.magic = htonl( DHCP_MAGIC );

	uint32_t yiaddr = dhcp_request_packet.yiaddr;

	request_payload.options[0] = DHCP_OPTION_TYPE;
	request_payload.options[1] = 1;
	request_payload.options[2] = DHCP_REQUEST;

	request_payload.options[3] = DHCP_OPTION_IP_REQUESTED;
	request_payload.options[4] = 4;
	request_payload.options[5] = yiaddr & 0xff;
	request_payload.options[6] = (yiaddr >> 8) & 0xff;
	request_payload.options[7] = (yiaddr >> 16) & 0xff;
	request_payload.options[8] = (yiaddr >> 24) & 0xff;

	request_payload.options[9] = DHCP_OPTION_PARAM_REQUEST_LIST;
	request_payload.options[10] = 2;
	request_payload.options[11] = 3;
	request_payload.options[12] = 6;

	request_payload.options[13] = 255;

	udp_send( DHCP_ADDR_BROADCAST, DHCP_DESTINATION_PORT, DHCP_SOURCE_PORT, (uint8_t *)&request_payload, sizeof( dhcp_packet ) + 14 );
}

void dhcp_handle_ack( dhcp_packet *packet, uint8_t *options ) {
	char ip_string[16];
	char subnet_string[16];
	char dns_string[16];
	char router_string[16];

	networking_info.ipv4_address = packet->yiaddr;
	ip_nota( htonl(packet->yiaddr), networking_info.ipv4_address_as_string );

	for( int i = 0; i < 255; ) {
		uint8_t option_type = options[i++];
		uint8_t option_size = options[i++];

		switch( option_type ) {
			case DHCP_OPTION_TYPE:
				break;
			case DHCP_OPTION_SERVER_IDENT:
				break;
			case DHCP_OPTION_SUBNET_MASK:
				memcpy( &networking_info.ipv4_subnet_mask, options + i, 4 );
				ip_nota( ntohl(networking_info.ipv4_subnet_mask), networking_info.ipv4_subnet_mask_as_string );

				break;
			case DHCP_OPTION_ROUTER:
				memcpy( &networking_info.ipv4_router, options + i, 4 );
				ip_nota( ntohl(networking_info.ipv4_router), networking_info.ipv4_router_as_string );

				break;
			case DHCP_OPTION_DNS:
				memcpy( &networking_info.ipv4_dns, options + i, 4 );
				ip_nota( ntohl(networking_info.ipv4_dns), networking_info.ipv4_dns_as_string );

				break;
			case DHCP_OPTION_IP_ADDR_LEASE:
				memcpy( &networking_info.dhcp_lease_time_in_seconds, options + i, 4 );

				break;
			case DHCP_OPTION_END:
				i = 255;

				break;
			default:
				i++;
		}

		i = i + option_size;
	}

	debugf( "DHCP Setup Complete:\n" );
	debugf( "    IP: %s\n", networking_info.ipv4_address_as_string );
	debugf( "    Subnet Mask: %s\n", networking_info.ipv4_subnet_mask_as_string);
	debugf( "    Router: %s\n", networking_info.ipv4_router_as_string );
	debugf( "    DNS: %s\n", networking_info.ipv4_dns_as_string );
}