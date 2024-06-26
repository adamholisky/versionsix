#include <kernel_common.h>
#include <net/ethernet.h>
#include <net/arp.h>

arp_packet packet;

extern "C" void arp_send( uint8_t *dest_protocol_addr ) {
	packet.htype = htons(0x0001);
	packet.ptype = htons(0x0800);
	packet.hlen = 0x6;
	packet.plen = 0x4;
	packet.opcode = htons(0x001);

	packet.src_hardware_addr[0] = 0x12;
	packet.src_hardware_addr[1] = 0x34;
	packet.src_hardware_addr[2] = 0x56;
	packet.src_hardware_addr[3] = 0x78;
	packet.src_hardware_addr[4] = 0x9A;
	packet.src_hardware_addr[5] = 0xBC;

	packet.src_protocol_addr[0] = 10;
	packet.src_protocol_addr[1] = 0;
	packet.src_protocol_addr[2] = 2;
	packet.src_protocol_addr[3] = 14;

	for( int i = 0; i < 6; i++ ) {
		packet.dst_hardware_addr[i] = 0xFF;
	}

	packet.dst_protocol_addr[0] = dest_protocol_addr[0];
    packet.dst_protocol_addr[1] = dest_protocol_addr[1];
    packet.dst_protocol_addr[2] = dest_protocol_addr[2];
    packet.dst_protocol_addr[3] = dest_protocol_addr[3];

	uint8_t dest[6];
	dest[0] = 0xFF;
	dest[1] = 0xFF;
	dest[2] = 0xFF;
	dest[3] = 0xFF;
	dest[4] = 0xFF;
	dest[5] = 0xFF;

	uint8_t *final_arp_packet = (uint8_t *)&packet;

	ethernet_send_packet( (uint8_t *)&dest, ETHERNET_TYPE_ARP, final_arp_packet, sizeof( arp_packet ) );
}

extern "C" void arp_process_packet( arp_packet *packet ) {
	debugf( "Got ARP Packet:\n" );
	debugf( "    Opcode:            0x%04X\n", htons(packet->opcode) );
	debugf( "    Source HW:         %X:%X:%X:%X:%X:%X\n", packet->src_hardware_addr[0], packet->src_hardware_addr[1], packet->src_hardware_addr[2], packet->src_hardware_addr[3], packet->src_hardware_addr[4], packet->src_hardware_addr[5] );
	debugf( "    Source IP:         %d.%d.%d.%d\n", packet->src_protocol_addr[0], packet->src_protocol_addr[1], packet->src_protocol_addr[2], packet->src_protocol_addr[3] );
	debugf( "    Destination HW:    %X:%X:%X:%X:%X:%X\n", packet->dst_hardware_addr[0], packet->dst_hardware_addr[1], packet->dst_hardware_addr[2], packet->dst_hardware_addr[3], packet->dst_hardware_addr[4], packet->dst_hardware_addr[5] );
	debugf( "    Destination IP:    %d.%d.%d.%d\n", packet->dst_protocol_addr[0], packet->dst_protocol_addr[1], packet->dst_protocol_addr[2], packet->dst_protocol_addr[3] );
}