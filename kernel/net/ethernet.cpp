#include <kernel_common.h>
#include <net/ethernet.h>
#include <e1000.h>

uint8_t packet_data[2048];

extern "C" void ethernet_send_packet( uint8_t *dest, uint16_t type, uint8_t *data, uint32_t length ) {
    ethernet_packet packet;

	packet.source[0] = 0x12;
	packet.source[1] = 0x34;
	packet.source[2] = 0x56;
	packet.source[3] = 0x78;
	packet.source[4] = 0x9A;
	packet.source[5] = 0xBC;

	packet.type = htons( type );

	for( int i = 0; i < 6; i++ ) {
		packet.destination[i] = dest[i];
	}

    memset( &packet_data, 0, 2048 );

    uint32_t total_size = sizeof( ethernet_packet ) + length;

	memcpy( &packet_data, (void *)&packet, sizeof( ethernet_packet ) );
	memcpy( (uint8_t *)&packet_data + sizeof( ethernet_packet ), (void *)data, length );

	kdebug_peek_at( (uint64_t)&packet_data );

	e1000_send( (uint8_t *)&packet_data, total_size );
}

extern "C" void ethernet_process_packet( uint64_t *data ) {
	ethernet_packet *packet = (ethernet_packet *)data;

	debugf( "Got Ethernet Packet:\n" );
	debugf( "    Type:        0x%04X\n", packet->type );
	debugf( "    Source:      %X:%X:%X:%X:%X:%X\n", packet->source[0], packet->source[1], packet->source[2], packet->source[3], packet->source[4], packet->source[5] );
	debugf( "    Destination: %X:%X:%X:%X:%X:%X\n", packet->destination[0], packet->destination[1], packet->destination[2], packet->destination[3], packet->destination[4], packet->destination[5] );
}