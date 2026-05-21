#ifndef VIOS_DEV_SERIALS_INCLUDED
#define VIOS_DEV_SERIALS_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <device.h>
#include <wait_queue.h>

void device_register_serial3( void );
void serial3_close( inode_id id );
void serial3_open( inode_id id );
uint8_t serial3_read( inode_id id, uint8_t * buff, uint64_t count, uint64_t offset );
void serial3_write( inode_id id, void *buff, size_t count, size_t offset );
void serial3_interrupt_handler( registers **reg );
uint8_t serial3_wq_ready( wait_queue *queue, pid_t pid, void *wq_data );


device *device_register_serial4( void );
void serial4_open( inode_id id );
void serial4_close( inode_id id );
uint8_t serial4_read( inode_id id, uint8_t *buff, uint64_t count, uint64_t offset );
void serial4_write( inode_id id, void *buff, size_t count, size_t offset );

#ifdef __cplusplus
}
#endif
#endif