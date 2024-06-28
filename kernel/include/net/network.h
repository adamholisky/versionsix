#ifndef VIOS_NET_NETWORK_INCLUDED
#define VIOS_NET_NETWORK_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
	uint32_t ipv4_address;
    char ipv4_address_as_string[16];

    uint32_t ipv4_dns;
    char ipv4_dns_as_string[16];

    uint32_t ipv4_router;
    char ipv4_router_as_string[16];
    
    uint32_t ipv4_subnet_mask;
    char ipv4_subnet_mask_as_string[16];

    uint32_t dhcp_lease_time_in_seconds;
} net_info;

#ifdef __cplusplus
}
#endif
#endif