#if !defined(PS2_KEYBOARD_H_INCLUDED)
#define PS2_KEYBOARD_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <ksymbols.h>
#include <lib/list.h>
#include <interrupt.h>
#include <process.h>
#include <wait_queue.h>
#include <vfs.h>

#define PS2_PORT_DATA 0x60
#define PS2_PORT_STATUS 0x64
#define PS2_PORT_CMD 0x64

#define PS2_CMD_READ_CONFIG 0x20
#define PS2_CMD_WRITE_CONFIG 0x60
#define PS2_CMD_DISABLE_PORT2 0xA7
#define PS2_CMD_ENABLE_PORT2 0xA8
#define PS2_CMD_TEST_PORT2 0xA9
#define PS2_CMD_SELF_TEST 0xAA
#define PS2_CMD_TEST_PORT1 0xAB
#define PS2_CMD_DISABLE_PORT1 0xAD
#define PS2_CMD_ENABLE_PORT1 0xAE
#define PS2_CMD_SEND_TO_SECOND_PORT 0xD4

#define PS2_CFG_IRQ_PORT1 0x01
#define PS2_CFG_IRQ_PORT2 0x02

#define PS2_DEVICE_RESET 0xFF



typedef struct {
	uint8_t scancode;
} scancode_queue_data;

typedef struct {
	registers **context;
	int fd;
	char *buff;
	size_t count;
} ps2_keyboard_wq_data;

void device_register_ps2_keyboard( void );
void ps2_keyboard_close( inode_id id );
void ps2_keyboard_open( inode_id id );
uint8_t ps2_keyboard_read( inode_id id, uint8_t* buff, uint64_t count, uint64_t offset );
void ps2_keyboard_write( inode_id id, void* buff, size_t count, size_t offset );
uint8_t ps2_keyboard_wq_ready( wait_queue* queue, pid_t pid, void *wq_data );
void ps2_keyboard_interrupt_handler( registers **reg );

void ps2_wait_for_input_buffer( void );
void ps2_wait_for_output_buffer( void );
void ps2_send_command( uint8_t command, uint8_t param );
uint8_t ps2_send_command_get_reply( uint8_t command );


#ifdef __cplusplus
}
#endif

#endif