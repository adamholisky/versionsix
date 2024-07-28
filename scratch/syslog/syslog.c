#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "syslog.h"

/*  Syslog format development + test for vios

    Compile and run:

    gcc -O0 -g syslog.c -o syslog && rm syslogtest.log && touch syslogtest.log && ./syslog

*/
int main( int argc, char *argv[] ) {
    FILE *fp = fopen( "syslogtest.log", "r+" );

    if( fp == NULL ) {
        printf( "Can't open syslogtest.log.\n" );
        return 1;
    }

    printf( "hi\n" );

    return 0;
}