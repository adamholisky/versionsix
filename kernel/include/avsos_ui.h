#if !defined(AVSOS_UI_H_INCLUDED)
#define AVSOS_UI_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <process.h>

void enable_gui( void );
void load_font_stuff( void );
void load_gui_stuff( void );
void main_console_putc( uint8_t c );
void main_console_set_cursor_visiblity( bool visible );
void main_console_blink_cursor( void );

#ifdef __cplusplus
}
#endif

#endif