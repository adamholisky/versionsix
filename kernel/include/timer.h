#ifndef VIOS_TIMER_INCLUDED
#define VIOS_TIMER_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "interrupt.h"

void timer_initalize( void );
void timer_handler( registers **context );
void timer_wait( uint8_t n );
uint32_t timer_get_system_counter( void );

#ifdef __cplusplus
}
#endif
#endif