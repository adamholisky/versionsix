#include <kernel_common.h>
#include <kshell_app.h>
#include <net/arp.h>

KSHELL_COMMAND( arptest, kshell_app_arptest_main )

int kshell_app_arptest_main( int argc, char *argv[] ) {
	//uint8_t dest[] = {127,0,0,2};
	//uint8_t dest[] = {192,168,12,1};
	uint8_t dest[] = {10,0,2,2};

	arp_send( (uint8_t *)&dest );

	return 0;
}