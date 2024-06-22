#include <kernel_common.h>
#include <serial.h>
#include <kshell.h>

extern void do_test_send( void );

void kshell( void ) {
    bool    keep_going = true;
    char    read_char = 0;

    while( keep_going ) {
        printf( "Version VI: " );
        read_char = serial_read_port( COM1 );

        printf( "%c", read_char );

        switch( read_char ) {
            case 'q':
                keep_going = false;
                break;
            case 't':
                do_test_send();
                break;
        }

        printf( "\n" );
    }

    printf( "Goodbye.\n" );
}