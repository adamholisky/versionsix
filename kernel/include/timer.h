#ifndef VIOS_TIMER_INCLUDED
#define VIOS_TIMER_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif


void timer_initalize( void );
void timer_handler( registers *context );

#ifdef __cplusplus
}
#endif
#endif