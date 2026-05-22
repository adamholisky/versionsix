#include <kernel_common.h>
#include <device.h>
#include <devices/ps2_keyboard.h>
#include <keyboard.h>
#include <wait_queue.h>
#include <vfs.h>
#include <kmemory.h>

device d_ps2_keyboard;
wait_queue *wq_ps2_keyboard;
avs_list *scancode_queue;

void device_register_ps2_keyboard( void ) {
	memset( &d_ps2_keyboard, 0, sizeof(device) );

	strcpy( d_ps2_keyboard.major_id, "ps2keyboard" );
	strcpy( d_ps2_keyboard.minor_id, "0" );

	d_ps2_keyboard.close = ps2_keyboard_close;
	d_ps2_keyboard.open = ps2_keyboard_open;
	d_ps2_keyboard.read = ps2_keyboard_read;
	d_ps2_keyboard.write = ps2_keyboard_write;

	wq_ps2_keyboard = wq_new( "ps2keyboard", ps2_keyboard_wq_ready );
	scancode_queue = avs_list_init();

	device_register( &d_ps2_keyboard );

	//__asm__ volatile("cli");

	ps2_send_command( PS2_CMD_DISABLE_PORT1, 0 );
	ps2_send_command( PS2_CMD_DISABLE_PORT2, 0 );

	// Clear the buffer
	while( inportb( PS2_PORT_STATUS ) & 1 ) {
		inportb( PS2_PORT_DATA );
	}

	uint8_t ps2_config = 0;
	ps2_config = ps2_send_command_get_reply( PS2_CMD_READ_CONFIG ); // read config
	klog( LOG_DEBUG, "ps2_config: %X", ps2_config );
	ps2_config = ps2_config | PS2_CFG_IRQ_PORT1 | PS2_CFG_IRQ_PORT2;
	klog( LOG_DEBUG, "ps2_config: %X", ps2_config );
	ps2_send_command( PS2_CMD_WRITE_CONFIG, ps2_config );

	uint8_t ps2_self_test = 0;
	ps2_self_test = ps2_send_command_get_reply( PS2_CMD_SELF_TEST );
	klog( LOG_DEBUG, "ps2_selftest: %X", ps2_self_test );

	interrupt_add_irq_handler( 1, ps2_keyboard_interrupt_handler );

	ps2_send_command( PS2_CMD_ENABLE_PORT1, 0 );

	//__asm__ volatile("sti");

	//while( true ) { ; }

	//do_immediate_shutdown();
}

void ps2_keyboard_close( inode_id id ) {
	// Intentionally blank
}

void ps2_keyboard_open( inode_id id ) {
	// Intentionally blank
}

uint8_t ps2_keyboard_read( inode_id id, uint8_t * buff, uint64_t count, uint64_t offset ) {
	uint8_t scancode = 0;
	char c = 0;
	
	debugf( "herp\n" );

	if( scancode_queue->size == 0 ) {
		debugf( "Added wqd\n" );
		ps2_keyboard_wq_data *wqd = kmalloc( sizeof(ps2_keyboard_wq_data) );

		wqd->fd = id;
		wqd->buff = buff;
		wqd->count = count;
		
		wq_add( wq_ps2_keyboard, process_get_current_proc_id(), wqd );
	} else {
		avs_node *n = scancode_queue->tail;
		scancode_queue_data *d = (scancode_queue_data *)n->data;

		c = keyboard_scancode_to_char( d->scancode );

		avs_list_free(scancode_queue, n);
		kfree(d);
	}

	return c;
}

void ps2_keyboard_write( inode_id id, void *buff, size_t count, size_t offset ) {
	// Intentionally blank
}

void ps2_keyboard_interrupt_handler( registers **reg ) {
	uint8_t status;
	uint8_t new_scancode;

	klog( LOG_DEBUG, "in it" );

	status = inportb(0x64);

	if( status & 0x01 ) {
		debugf( "going ready\n" );
		new_scancode = inportb(0x60);

		scancode_queue_data *d = kmalloc( sizeof(scancode_queue_data) );
		d->scancode = new_scancode;

		avs_list_enqueue( scancode_queue, d );
		wq_make_ready( wq_ps2_keyboard );
		wq_call_next_ready( wq_ps2_keyboard );
	} else {
		klog( LOG_DEBUG, "Status & 0x01 failed." );
	}
}

uint8_t ps2_keyboard_wq_ready( wait_queue *queue, pid_t pid, void *wq_data ) {
	debugf( "In ready\n" );

	char c;
	ps2_keyboard_wq_data *wqd = (ps2_keyboard_wq_data *)wq_data;

	process_data *p = process_get_data_from_pid( pid );

	scancode_queue_data *d = avs_list_dequeue( scancode_queue );

	c = keyboard_scancode_to_char(d->scancode);
	*wqd->buff = c;
	p->status = PROCESS_STATUS_INACTIVE;
	klog(LOG_INFO, "Got a c: %d", c);

	kfree(d);

	return c;
}

void ps2_wait_for_input_buffer( void ) {
	uint64_t x = 1000000;

	do {
		if( inportb(PS2_PORT_STATUS) & (1 << 1 ) ) {
			return;
		}

		x--;
	} while( x != 0 );
}

void ps2_wait_for_output_buffer( void ) {
	uint64_t x = 1000000;

	do {
		if( inportb(PS2_PORT_STATUS) & (1 << 0 ) ) {
			return;
		}

		x--;
	} while( x != 0 );
}


void ps2_send_command( uint8_t command, uint8_t param ) {
	ps2_wait_for_input_buffer();
	outportb( PS2_PORT_CMD, command );

	if( param != 0 ) {
		ps2_wait_for_input_buffer();
		outportb( PS2_PORT_DATA, param );
	}
}

uint8_t ps2_send_command_get_reply( uint8_t command ) {
	ps2_wait_for_input_buffer();
	outportb( PS2_PORT_CMD, command );
	ps2_wait_for_output_buffer();
	return inportb( PS2_PORT_DATA );
}