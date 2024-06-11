#include <kernel_common.h>
#include <serial.h>
#include <kshell.h>

void kshell( void ) {
    bool    keep_going = true;
    char    read_char = 0;

    while( keep_going ) {
        printf( "Version VI: " );
        read_char = serial_read_port( COM1 );

        printf( "%c", read_char );

        if( read_char == 'q' ) {
            keep_going = false;
        }

        printf( "\n" );
    }

    printf( "Goodbye.\n" );
}