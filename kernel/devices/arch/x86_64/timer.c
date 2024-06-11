#include "kernel_common.h"
#include "interrupt.h"
#include "timer.h"

uint32_t timer_counter;

void timer_initalize( void ) {
    irq_handler_func timer_func = timer_handler;
    interrupt_add_irq_handler( 0, timer_func );

    uint16_t divisor = 11931;      // Calculate our divisor, default 65535 --> 1193180/hz
    outportb( 0x43, 0x36 );             // Set our command byte 0x36
    outportb( 0x40, divisor & 0xFF );   // Set low byte of divisor
    outportb( 0x40, divisor >> 8 );     // Set high byte of divisor

    timer_counter = 0;
}

void timer_handler( registers *context ) {
    if( timer_counter < 101 ) { 
        timer_counter++;
    } else {
        timer_counter = 0;
    }

    if( timer_counter == 0 ) {
        debugf( "Timer count hit.\n" );
    }
}