#ifndef VIOS_DEV_STDIN_INCLUDED
#define VIOS_DEV_STDIN_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <device.h>

void device_register_stdin( void );
void stdin_close( inode_id id );
void stdin_open( inode_id id );
uint8_t stdin_read( inode_id id, uint8_t * buff, uint64_t count, uint64_t offset );
void stdin_write( inode_id id, void *buff, size_t count, size_t offset );
void stdin_set_redirect( device *redirect_to );

#ifdef __cplusplus
}
#endif
#endif