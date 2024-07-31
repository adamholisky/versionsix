#ifndef VIOS_DEV_STDERR_INCLUDED
#define VIOS_DEV_STDERR_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <device.h>

void device_register_stderr( void );
void stderr_close( void );
void stderr_open( void );
uint8_t stderr_read( void );
void stderr_write( void *buff, size_t count );

#ifdef __cplusplus
}
#endif
#endif