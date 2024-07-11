#include "kernel_common.h"
#include "interrupt.h"
#include "timer.h"

uint32_t timer_counter;
bool done_waiting;

void timer_initalize( void ) {
    irq_handler_func timer_func = timer_handler;
    interrupt_add_irq_handler( 0, timer_func );

    uint16_t divisor = 11931;      // Calculate our divisor, default 65535 --> 1193180/hz
    outportb( 0x43, 0x36 );             // Set our command byte 0x36
    outportb( 0x40, divisor & 0xFF );   // Set low byte of divisor
    outportb( 0x40, divisor >> 8 );     // Set high byte of divisor

    timer_counter = 0;
    done_waiting = false;
}

void timer_handler( registers **context ) {
    // These numbers are awful. This whole thing needs to be made to not be dumb.

    if( timer_counter == 50 ) {
        main_console_blink_cursor();
    } else if( timer_counter == 100 ) {
        main_console_blink_cursor();
    }

    if( timer_counter < 101 ) { 
        timer_counter++;
    } else {
        timer_counter = 0;
    }

    if( timer_counter == 0 ) {
        done_waiting = true;
    }
}

void timer_wait( uint8_t n ) {
    uint8_t wait_count = 0;

    done_waiting = false;

    while( wait_count != n ) {
        if( done_waiting ) {
            wait_count++;
            done_waiting = false;
            //dfv( wait_count );
        }
    }
}