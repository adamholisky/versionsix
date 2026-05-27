#include <kernel_common.h>
#include <device.h>
#include <devices/ps2.h>
#include <keyboard.h>
#include <wait_queue.h>
#include <vfs.h>
#include <kmemory.h>
#include <vui/vui.h>

device d_ps2_keyboard;
device d_ps2_mouse;

wait_queue *wq_ps2_keyboard;
avs_list *scancode_queue;

volatile unsigned int mouse_cycle = 0;
uint8_t mouse_byte[ 3 ];
bool middle_button = false;
bool right_button = false;
bool left_button = false;

void device_register_ps2_keyboard( void ) {
	memset( &d_ps2_keyboard, 0, sizeof(device) );
	memset( &d_ps2_mouse, 0, sizeof(device) );

	// Keyboard setup
	strcpy( d_ps2_keyboard.major_id, "ps2keyboard" );
	strcpy( d_ps2_keyboard.minor_id, "0" );

	d_ps2_keyboard.close = ps2_keyboard_close;
	d_ps2_keyboard.open = ps2_keyboard_open;
	d_ps2_keyboard.read = ps2_keyboard_read;
	d_ps2_keyboard.write = ps2_keyboard_write;

	wq_ps2_keyboard = wq_new( "ps2keyboard", ps2_keyboard_wq_ready );
	scancode_queue = avs_list_init();

	device_register( &d_ps2_keyboard );

	// Mouse device setup
	strcpy( d_ps2_mouse.major_id, "ps2mouse" );
	strcpy( d_ps2_mouse.minor_id, "0" );

	d_ps2_mouse.close = ps2_mouse_close;
	d_ps2_mouse.open = ps2_mouse_open;
	d_ps2_mouse.read = ps2_mouse_read;
	d_ps2_mouse.write = ps2_mouse_write;

	device_register( &d_ps2_mouse );

	__asm__ volatile("cli");

	mouse_byte[0] = 0;
	mouse_byte[1] = 0;
	mouse_byte[2] = 0;
	mouse_cycle = 0;

	ps2_wait_for_input_buffer();
	outportb( PS2_PORT_CMD, PS2_CMD_DISABLE_PORT1 );

	ps2_wait_for_input_buffer();
	outportb( PS2_PORT_CMD, PS2_CMD_DISABLE_PORT2 );

	// Clear the buffer
	while( inportb( PS2_PORT_STATUS ) & 1 ) {
		inportb( PS2_PORT_DATA );
	}

	// PS2 I8042: Read the config
	uint8_t ps2_config = 0;
	ps2_wait_for_input_buffer();
	outportb( PS2_PORT_CMD, PS2_CMD_READ_CONFIG );
	ps2_wait_for_input_buffer();
	ps2_config = inportb( PS2_PORT_DATA );

	// PS2 I8042: Setup the config as we want it
	ps2_config = ps2_config | PS2_CFG_IRQ_PORT1 | PS2_CFG_IRQ_PORT2;
	df( "p2config: %X\n", ps2_config );

	// PS2 I8042: Write new config back
	ps2_wait_for_input_buffer();
	outportb( PS2_PORT_CMD, PS2_CMD_WRITE_CONFIG );
	ps2_wait_for_input_buffer();
	outportb( PS2_PORT_DATA, ps2_config );

	// Mouse: use defaults
	ps2_wait_for_input_buffer();
	outportb( PS2_PORT_CMD, PS2_CMD_SEND_TO_SECOND_PORT );
	ps2_wait_for_input_buffer();
	outportb( PS2_PORT_DATA, PS2_CMD_MOUSE_USE_DEFAULT_SETTINGS );
	inportb( PS2_PORT_DATA ); // ack

	// Mouse: enable
	ps2_wait_for_input_buffer();
	outportb( PS2_PORT_CMD, PS2_CMD_SEND_TO_SECOND_PORT );
	ps2_wait_for_input_buffer();
	outportb( PS2_PORT_DATA, PS2_CMD_ENABLE_MOUSE );
	inportb( PS2_PORT_DATA ); // ack

	// Do a self test for good measure, should be 0x55
	uint8_t ps2_self_test = 0;

	ps2_wait_for_input_buffer();
	outportb( PS2_PORT_CMD, PS2_CMD_SELF_TEST );
	ps2_wait_for_input_buffer();
	ps2_self_test = inportb( PS2_PORT_DATA );
	if( ps2_self_test != 0x55 ) {
		klog( LOG_PANIC, "ps2_selftest returned 0x%X, expected 0x55.", ps2_self_test );
	}

	// Add our interrupt handlers
	interrupt_add_irq_handler( 1, ps2_keyboard_interrupt_handler );
	interrupt_add_irq_handler( 12, ps2_mouse_interrupt_handler );

	// PS2 I8042: Enable ports
	ps2_wait_for_input_buffer();
	outportb( PS2_PORT_CMD, PS2_CMD_ENABLE_PORT1 );

	ps2_wait_for_input_buffer();
	outportb( PS2_PORT_CMD, PS2_CMD_ENABLE_PORT2 );

	__asm__ volatile("sti");

	//while( true ) { ; }

	//do_immediate_shutdown();
}

/***************************************************/
/* Universal PS2 routines                          */
/***************************************************/

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

/***************************************************/
/* PS2 Keyboard support                            */
/***************************************************/

void ps2_keyboard_close( inode_id id ) {
	// Intentionally blank
}

void ps2_keyboard_open( inode_id id ) {
	// Intentionally blank
}

void ps2_keyboard_write( inode_id id, void *buff, size_t count, size_t offset ) {
	// Intentionally blank
}


uint8_t ps2_keyboard_read( inode_id id, uint8_t * buff, uint64_t count, uint64_t offset ) {
	uint8_t scancode = 0;
	char c = 0;
	
	if( scancode_queue->size == 0 ) {
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

void ps2_keyboard_interrupt_handler( registers **reg ) {
	uint8_t status;
	uint8_t new_scancode;

	status = inportb(0x64);

	if( status & 0x01 ) {
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
	if( wq_data == NULL ) {
		klog( LOG_PANIC, "wq_data is null" );
		return 0;
	}

	char c;
	ps2_keyboard_wq_data *wqd = (ps2_keyboard_wq_data *)wq_data;

	process_data *p = process_get_data_from_pid( pid );

	scancode_queue_data *d = avs_list_dequeue( scancode_queue );

	c = keyboard_scancode_to_char(d->scancode);
	*wqd->buff = c;
	p->status = PROCESS_STATUS_INACTIVE;

	kfree(d);

	return c;
}

/***************************************************/
/* PS2 Mouse support                               */
/***************************************************/

void ps2_mouse_close( inode_id id ) {
	// Intentionally blank
}

void ps2_mouse_open( inode_id id ) {
	// Intentionally blank
}

void ps2_mouse_write( inode_id id, void *buff, size_t count, size_t offset ) {
	// Intentionally blank
}

uint8_t ps2_mouse_read( inode_id id, uint8_t * buff, uint64_t count, uint64_t offset ) {
	// Intentionally blank
}

void ps2_mouse_interrupt_handler( registers **reg ) {
	int move_x = 0;
	int move_y = 0;

	uint8_t in_byte = inportb( PS2_PORT_DATA );

	if( mouse_cycle == 0 ) {
		if( !(in_byte & (1 << 3)) ) {
			klog( LOG_PANIC, "3rd bit not set, oops?" );
			return;
		}
	}

	mouse_byte[ mouse_cycle++ ] = in_byte;

	if( mouse_cycle == 3 ) {
		move_x = mouse_byte[1];
		move_y = mouse_byte[2];
		
		if( mouse_byte[0] & (1 << 4) ) { move_x = move_x - 0x100; }
		if( mouse_byte[0] & (1 << 5) ) { move_y = move_y - 0x100; }

		if( mouse_byte[0] & (1 << 6) || mouse_byte[0] & (1 << 7) ) { 
			move_x = 0; 
			move_y = 0;
		}

		mouse_cycle = 0;
		middle_button = false;
		right_button = false;
		left_button = false;

		if( mouse_byte[0] & 0x4 ) {
			middle_button = true;
		}

		if( mouse_byte[0] & 0x2 ) {
			right_button = true;
		}

		if( mouse_byte[0] & 0x1 ) {
			left_button = true;
			debugf( "At 3: (%d, %d) with M: %d, R: %d, L: %d\n", move_x, move_y, middle_button, right_button, left_button );
		}

		//printf( "(%d, %d) ", move_x, move_y );

		vui_handle_mouse_move( move_x, move_y * -1 );
	}
}