#ifndef VIOS_SERIAL_INCLUDED
#define VIOS_SERIAL_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define serial_use_default_port 0

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

#define serial_write( c ) serial_write_port( c, COM1 )
#define serial_read( ) serial_read_port( serial_use_default_port )

void serial_initalize( void );
void serial_setup_port( uint32_t port );
void serial_set_default_port( uint32_t port );
void serial_write_port( char c, uint32_t port );
char serial_read_port( uint32_t port );

#ifdef __cplusplus
}
#endif
#endif