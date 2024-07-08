#include <kernel_common.h>
#include <net/network.h>
#include <net/tcp.h>
#include <net/ipv4.h>
#include <timer.h>

extern net_info networking_info;

tcp_connection test_connection;

void tcp_test( void );

void tcp_test( void ) {
	memset( &test_connection, 0, sizeof( tcp_connection ) );

	// NIST Day/Time Server
	/* connection->source_port = 5894;
	connection->dest_port = 13;
	connection->dest_ip = ip_to_uint(192,6,15,28); */

	// Blizzard Watch
	test_connection.source_port = 5894;
	test_connection.dest_port = 2000;
	test_connection.dest_ip = ip_to_uint(172,23,35,254);
	//connection->dest_ip = ip_to_uint(10,0,2,100);

	tcp_connection_connect( &test_connection );
	timer_wait( 1 );

	char data_to_send[] = "Hello, world!";
	tcp_connection_send_data( &test_connection, (uint8_t *)data_to_send, sizeof( data_to_send ) );
	timer_wait( 1 );

	do_immediate_shutdown();
}

uint16_t calculate_tcp_checksum( tcp_check_header *p, uint16_t *h, uint16_t options_length, void *d, uint32_t payload_size ) {
	uint32_t sum = 0;
	uint16_t * s = (uint16_t *)p;

	for (int i = 0; i < 6; ++i) {
		sum += ntohs(s[i]);
		if (sum > 0xFFFF) {
			sum = (sum >> 16) + (sum & 0xFFFF);
		}
	}

	s = (uint16_t *)h;
	for (int i = 0; i < (10 + options_length/2); ++i) {
		sum += ntohs(s[i]);
		if (sum > 0xFFFF) {
			sum = (sum >> 16) + (sum & 0xFFFF);
		}
	}

	uint16_t d_words = payload_size / 2;

	s = (uint16_t *)d;
	for (unsigned int i = 0; i < d_words; ++i) {
		sum += ntohs(s[i]);
		if (sum > 0xFFFF) {
			sum = (sum >> 16) + (sum & 0xFFFF);
		}
	}

	if (d_words * 2 != payload_size) {
		uint8_t * t = (uint8_t *)d;
		uint8_t tmp[2];
		tmp[0] = t[d_words * sizeof(uint16_t)];
		tmp[1] = 0;

		uint16_t * f = (uint16_t *)tmp;

		sum += ntohs(f[0]);
		if (sum > 0xFFFF) {
			sum = (sum >> 16) + (sum & 0xFFFF);
		}
	}

	return ~(sum & 0xFFFF) & 0xFFFF;
}

void tcp_process_packet( uint8_t *data, uint16_t length ) {
	// TODO: do multiple connections better
	tcp_connection_handle_recv( &test_connection, data, length );
}

void tcp_send_packet( uint32_t dest, uint16_t dest_port, uint16_t src_port, uint8_t *data, uint16_t length ) {
	log_entry_enter();

	log_entry_exit();
}

void tcp_send_header_and_data( uint32_t dest, uint16_t dest_port, uint16_t src_port, tcp_header *header, uint8_t *data, uint16_t length ) {
	tcp_check_header checksum_header;

	checksum_header.destination = htonl(dest);
	checksum_header.source = networking_info.ipv4_address;
	checksum_header.protocol = IPV4_PROTOCOL_TCP;
	checksum_header.zeros = 0;
	checksum_header.tcp_len = htons(sizeof( tcp_header ) + length);

	uint32_t computed_checksum = calculate_tcp_checksum( &checksum_header, (uint16_t *)header, 0, data, length );
	header->checksum = htons( computed_checksum );

	uint8_t payload[2048];
	memcpy( payload, header, sizeof( tcp_header ) );
	memcpy( payload + sizeof(tcp_header), data, length );

	ipv4_send( dest, IPV4_PROTOCOL_TCP, payload, sizeof( tcp_header ) + length );
}

void tcp_send_header( uint32_t dest, uint16_t dest_port, uint16_t src_port, tcp_header *header ) {
	tcp_check_header checksum_header;

	checksum_header.destination = htonl(dest);
	checksum_header.source = networking_info.ipv4_address;
	checksum_header.protocol = IPV4_PROTOCOL_TCP;
	checksum_header.zeros = 0;
	checksum_header.tcp_len = htons(sizeof( tcp_header ));

	uint32_t computed_checksum = calculate_tcp_checksum( &checksum_header, (uint16_t *)header, 0, NULL, 0 );
	header->checksum = htons( computed_checksum );

	ipv4_send( dest, IPV4_PROTOCOL_TCP, (uint8_t *)header, sizeof( tcp_header ) );
}

void tcp_send_header_and_options( uint32_t dest, uint16_t dest_port, uint16_t src_port, tcp_header_payload *payload, uint16_t options_size ) {
	tcp_check_header checksum_header;

	checksum_header.destination = htonl(dest);
	checksum_header.source = networking_info.ipv4_address;
	checksum_header.protocol = IPV4_PROTOCOL_TCP;
	checksum_header.zeros = 0;
	checksum_header.tcp_len = htons(sizeof( tcp_header ) + options_size);

	uint32_t computed_checksum = calculate_tcp_checksum( &checksum_header, (uint16_t *)payload, options_size, NULL, 0 );
	payload->header.checksum = htons( computed_checksum );

	ipv4_send( dest, IPV4_PROTOCOL_TCP, (uint8_t *)payload, sizeof( tcp_header ) + options_size );
}

void tcp_connection_connect( tcp_connection *con ) {
	tcp_header_payload payload;

	memset( &payload, 0, sizeof( tcp_header_payload ) );

	con->connected = false;
	con->connection_state = 0;
	con->seq = 159753100;
	con->ack = 0;

	payload.header.ack_number = 0;
	payload.header.seq_number = htonl(con->seq);
	payload.header.destination_port = htons( con->dest_port );
	payload.header.source_port = htons( con->source_port );
	
	payload.header.window_size = htons( 64240 );
	payload.header.checksum = 0;
	payload.header.urgent = 0;

	payload.options[0] = 2; //max segment size
	payload.options[1] = 4;
	payload.options[2] = 0x05;
	payload.options[3] = 0xb4;
	payload.options[4] = 4; //SACK permitted
	payload.options[5] = 2;
	payload.options[6] = 3; //window scale
	payload.options[7] = 3;
	payload.options[8] = 7;
	payload.options[9] = 1; //nop
	payload.options[10] = 1;
	payload.options[11] = 1;

	uint16_t options_size = 12;

	uint16_t header_total_size = sizeof(tcp_header) + options_size;
	uint16_t header_length_as_uint32 = header_total_size / 4;
	uint16_t header_length_adjusted = header_length_as_uint32 << 12;

	payload.header.flags = htons( TCP_FLAG_SYN | header_length_adjusted ); // SYN

	tcp_connection_send_header_and_options( con, &payload, 12 );

	// Do this much better...
	while( !con->connected ) {
		switch( con->connection_state ) {
			case CONNECTION_SYN_SENT:
				// TODO: exter after x ticks
				break;
			case CONNECTION_SYN_ACK_RECV:
				tcp_connection_send_ack( con );
				con->connected = true;
				break;
		}
	}

	debugf( "Connection established!\n" );
}

void tcp_connection_send_header_and_options( tcp_connection *con, tcp_header_payload *payload, uint16_t options_size ) {
	tcp_send_header_and_options( con->dest_ip, con->dest_port, con->source_port, payload, 12 );
}

void tcp_connection_send_header( tcp_connection *con, tcp_header *header ) {
	tcp_send_header( con->dest_ip, con->dest_port, con->source_port, header );
}

void tcp_connection_send_ack( tcp_connection *con ) {
	tcp_header header;

	memset( &header, 0, sizeof( tcp_header ) );

	con->ack = con->ack + 1;

	header.ack_number = htonl(con->ack);
	header.seq_number = htonl(con->seq);
	header.destination_port = htons( con->dest_port );
	header.source_port = htons( con->source_port );
	header.window_size = htons( 64240 );
	header.checksum = 0;
	header.urgent = 0;
	header.flags = htons(( TCP_FLAG_ACK | 0x5000 ));

	tcp_connection_send_header( con, &header );
}

void tcp_connection_send_data( tcp_connection *con, uint8_t *data, uint16_t length ) {
	tcp_header header;

	memset( &header, 0, sizeof( tcp_header ) );

	header.ack_number = htonl(con->ack);
	header.seq_number = htonl(con->seq);
	header.destination_port = htons( con->dest_port );
	header.source_port = htons( con->source_port );
	header.window_size = htons( 64240 );
	header.checksum = 0;
	header.urgent = 0;
	header.flags = htons(( TCP_FLAG_PSH | TCP_FLAG_ACK | 0x5000 ));

	tcp_send_header_and_data( con->dest_ip, con->dest_port, con->source_port, &header, data, length );
}

void tcp_connection_handle_recv( tcp_connection *con, uint8_t *data, uint16_t length ) {
	tcp_header *header = (tcp_header *)data;
	uint8_t *payload_data = NULL;
	uint16_t payload_data_length = 0;

	if( length > sizeof( tcp_header ) ) {
		// has data
		payload_data = data + sizeof( tcp_header );
		payload_data_length = length - sizeof( tcp_header );
	}

	con->ack = htonl(header->seq_number);
	con->seq = htonl(header->ack_number);

	uint16_t flags = htons(header->flags) & 0x0FFF;

	switch( flags ) {
		case (TCP_FLAG_SYN | TCP_FLAG_ACK):
			debugf( "Got SYN | ACK\n" );
			con->connection_state = CONNECTION_SYN_ACK_RECV;
			payload_data_length = 0;
			break;
		case TCP_FLAG_PSH | TCP_FLAG_ACK:
			debugf( "Got PSH | ACK\n" );
			break;
		case TCP_FLAG_ACK:
			debugf( "Got ACK\n" );
			payload_data_length = 0;
			break;
	}

	if( payload_data_length > 0 ) {
		char final_data[2048];
		memset( final_data, 0, 2048 );
		memcpy( final_data, payload_data, payload_data_length );
		debugf( "Got data: \"%s\"\n", final_data );
	}
}