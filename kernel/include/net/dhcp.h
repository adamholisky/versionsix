#ifndef VIOS_NET_DHCP_INCLUDED
#define VIOS_NET_DHCP_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define DHCP_ADDR_BROADCAST 0xFFFFFFFF

#define DHCP_MAGIC 0x63825363

#define DHCP_SOURCE_PORT 68
#define DHCP_DESTINATION_PORT 67

#define DHCP_DISCOVER 1
#define DHCP_OFFER 2
#define DHCP_REQUEST 3
#define DHCP_DECLINE 4
#define DHCP_ACK 5
#define DHCP_NAK 6
#define DHCP_RELEASE 7
#define DHCP_INFORM 8
#define DHCP_LEASEQUERY 10
#define DHCP_LEASEUNASSIGNED 11
#define DHCP_LEASEUNKNOWN 12
#define DHCP_LEASEACTIVE 13

#define DHCP_BOOT_REPLY 2

#define DHCP_OPTION_TYPE 53
#define DHCP_OPTION_PARAM_REQUEST_LIST 55
#define DHCP_OPTION_IP_REQUESTED 50
#define DHCP_OPTION_SUBNET_MASK 1
#define DHCP_OPTION_END 255
#define DHCP_OPTION_ROUTER 3
#define DHCP_OPTION_DNS 6
#define DHCP_OPTION_IP_ADDR_LEASE 51
#define DHCP_OPTION_SERVER_IDENT 54

typedef struct {
	uint8_t op;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;

	uint32_t xid;

	uint16_t secs;
	uint16_t flags;

	uint32_t ciaddr;
	uint32_t yiaddr;
	uint32_t siaddr;
	uint32_t giaddr;

	uint8_t  chaddr[16];

	uint8_t sname[64];
	uint8_t file[128];

	uint32_t magic;
} __attribute__ ((packed)) dhcp_packet;

typedef struct {
    dhcp_packet header;
    uint8_t options[32];
} __attribute__ ((packed)) dhcp_payload;

void dhcp_start( void );
void dhcp_process_packet( uint8_t *unprocessed_data );

void dhcp_handle_offer( dhcp_packet *packet );
void dhcp_handle_offer_stage_2( void );
void dhcp_handle_ack( dhcp_packet *packet, uint8_t *options );

#ifdef __cplusplus
}
#endif
#endif