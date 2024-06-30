#ifndef VIOS_NET_TCP_INCLUDED
#define VIOS_NET_TCP_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define TCP_FLAG_FIN (1 << 0)
#define TCP_FLAG_SYN (1 << 1)
#define TCP_FLAG_RESET (1 << 2)
#define TCP_FLAG_PSH (1 << 3)
#define TCP_FLAG_ACK (1 << 4)

#define CONNECTION_SYN_SENT 1
#define CONNECTION_SYN_ACK_RECV 2

typedef struct {
	uint16_t source_port;
	uint16_t destination_port;

	uint32_t seq_number;
	uint32_t ack_number;

	uint16_t flags;
	uint16_t window_size;
	uint16_t checksum;
	uint16_t urgent;
} __attribute__((aligned(2))) __attribute__((packed)) tcp_header;

typedef struct {
	uint32_t source;
	uint32_t destination;
	uint8_t  zeros;
	uint8_t  protocol;
	uint16_t tcp_len;
} __attribute__((packed)) tcp_check_header;

typedef struct {
	tcp_header header;
	uint8_t options[32];
} __attribute__((packed)) tcp_header_payload;

class TCP_Connection {
    public:
        uint16_t source_port;
        uint16_t dest_port;
        uint32_t dest_ip;
		bool connected;
	
        void connect( void );

		//make these private later?
		uint32_t seq;
		uint32_t ack;
		uint8_t connection_state;
		void send_header_and_options( tcp_header_payload *payload, uint16_t options_size );
		void send_header( tcp_header *header );
		void handle_recv( uint8_t *data, uint16_t length );
		void send_ack( void );
		void send_data( uint8_t *data, uint16_t length );
};

void tcp_process_packet( uint8_t *data, uint16_t length );
void tcp_send_header( uint32_t dest, uint16_t dest_port, uint16_t src_port, tcp_header *header );
void tcp_send_header_and_options( uint32_t dest, uint16_t dest_port, uint16_t src_port, tcp_header_payload *payload, uint16_t options_size );
void tcp_send_header_and_data( uint32_t dest, uint16_t dest_port, uint16_t src_port, tcp_header *header, uint8_t *data, uint16_t length );

uint16_t calculate_tcp_checksum( tcp_check_header *p, uint16_t *h, uint16_t options_length, void *d, uint32_t payload_size );

#ifdef __cplusplus
}
#endif
#endif