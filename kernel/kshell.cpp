#include <kernel_common.h>
#include <serial.h>
#include <kshell.h>
#include <keyboard.h>
#include <net/arp.h>

extern void do_test_send( void );

void kshell( void ) {
    bool    keep_going = true;
    char    read_char = 0;

    while( keep_going ) {
        printf( "Version VI: " );
        read_char = keyboard_get_char();;

        printf( "%c", read_char );

        switch( read_char ) {
            case 'q':
                keep_going = false;
                break;
            case 't':
                uint8_t dest[] = {10,0,2,2};

	            arp_send( (uint8_t *)&dest );
                break;
        }

        printf( "\n" );
    }

    printf( "Goodbye.\n" );
}