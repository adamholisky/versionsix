#include <kernel_common.h>
#include <net/udp.h>
#include <net/ipv4.h>
#include <net/ethernet.h>
#include <net/dhcp.h>

void udp_send( uint32_t dest_ip, uint16_t dest_port, uint16_t src_port, uint8_t *data, uint16_t length ) {
    udp_packet packet;
    uint8_t payload[2048];

    memset( payload, 0, 2048 );

    packet.source_port = htons(src_port);
    packet.destination_port = htons(dest_port);
    packet.length = htons(sizeof( udp_packet ) + length);
    packet.checksum = 0;

    memcpy( payload, &packet, sizeof( udp_packet ) );
    memcpy( (payload + sizeof( udp_packet )), data, length );

    //kdebug_peek_at( (uint64_t)payload );

    ipv4_send( dest_ip, IPV4_PROTOCOL_UDP, payload, sizeof( udp_packet ) + length );
}

void udp_process_packet( uint8_t *unprocessed_data ) {
    udp_packet *packet = (udp_packet *)unprocessed_data;
    uint8_t *data = (uint8_t *)unprocessed_data + sizeof( udp_packet );

    /* debugf( "Got UDP Packet:\n" );
    debugf( "    Src port:  0x%X (%d)\n", htons(packet->source_port), htons(packet->source_port) );
    debugf( "    Dest Port: 0x%X (%d)\n", htons(packet->destination_port), htons(packet->destination_port) ); */

    switch( htons(packet->destination_port) ) {
        case 68:
            dhcp_process_packet( data );
            break;
        default:
            debugf( "UDP Unhandled port: %d\n", htons(packet->destination_port) );
    }
}