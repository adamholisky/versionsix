#include <kernel_common.h>
#include <kmemory.h>
#include <net/arp.h>
#include <net/ethernet.h>
#include <net/dhcp.h>
#include <net/network.h>
#include <e1000.h>

net_info networking_info;

void enable_networking( void ) {
	memset( &networking_info, 0, sizeof( net_info ) );
	e1000_initalize();
	dhcp_start();
}