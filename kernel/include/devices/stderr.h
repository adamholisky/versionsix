#ifndef VIOS_DEV_STDERR_INCLUDED
#define VIOS_DEV_STDERR_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <device.h>

void device_register_stderr( void );
void stderr_close( inode_id id );
void stderr_open( inode_id id );
uint8_t stderr_read( inode_id id, uint8_t * buff, uint64_t count, uint64_t offset );
void stderr_write( inode_id id, void *buff, size_t count, size_t offset );

#ifdef __cplusplus
}
#endif
#endif