#include <kernel_common.h>
#include <interrupt.h>
#include <keyboard.h>
#include <task.h>

keyboard_config main_keyboard;

unsigned char keyboard_map[128] = {
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8',	  /* 9 */
	'9', '0', '-', '=', '\b',						  /* Backspace */
	'\t',											  /* Tab */
	'q', 'w', 'e', 'r',								  /* 19 */
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	  /* Enter key */
	0,												  /* 29   - Control */
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 39 */
	'\'', '`', 0,									  /* Left shift */
	'\\', 'z', 'x', 'c', 'v', 'b', 'n',				  /* 49 */
	'm', ',', '.', '/', 0,							  /* Right shift */
	'*',
	0,	 /* Alt */
	' ', /* Space bar */
	0,	 /* Caps lock */
	0,	 /* 59 - F1 key ... > */
	0, 0, 0, 0, 0, 0, 0, 0,
	0, /* < ... F10 */
	0, /* 69 - Num lock*/
	0, /* Scroll Lock */
	0, /* Home key */
	0, /* Up Arrow */
	0, /* Page Up */
	'-',
	0, /* Left Arrow */
	0,
	0, /* Right Arrow */
	'+',
	0, /* 79 - End key*/
	0, /* Down Arrow */
	0, /* Page Down */
	0, /* Insert Key */
	0, /* Delete Key */
	0, 0, 0,
	0, /* F11 Key */
	0, /* F12 Key */
	0, /* All other keys are undefined */
};

unsigned char keyboard_map_shift[128] = {
	0, 0, '!', '@', '#', '$', '%', '^', '&', '*',	  /* 9 */
	'(', ')', '_', '+', '\b',						  /* Backspace */
	'\t',											  /* Tab */
	'Q', 'W', 'E', 'R',								  /* 19 */
	'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	  /* Enter key */
	0,												  /* 29   - Control */
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', /* 39 */
	'"', '~', 0,									  /* Left shift */
	'|', 'Z', 'X', 'C', 'V', 'B', 'N',				  /* 49 */
	'M', '<', '>', '?', 0,							  /* Right shift */
	'*',
	0,	 /* Alt */
	' ', /* Space bar */
	0,	 /* Caps lock */
	0,	 /* 59 - F1 key ... > */
	0, 0, 0, 0, 0, 0, 0, 0,
	0, /* < ... F10 */
	0, /* 69 - Num lock*/
	0, /* Scroll Lock */
	0, /* Home key */
	0, /* Up Arrow */
	0, /* Page Up */
	'-',
	0, /* Left Arrow */
	0,
	0, /* Right Arrow */
	'+',
	0, /* 79 - End key*/
	0, /* Down Arrow */
	0, /* Page Down */
	0, /* Insert Key */
	0, /* Delete Key */
	0, 0, 0,
	0, /* F11 Key */
	0, /* F12 Key */
	0, /* All other keys are undefined */
};

void keyboard_initalize( void ) {
	main_keyboard.is_waiting = false;
	main_keyboard.is_shift = false;
	main_keyboard.in_E0 = false;
	main_keyboard.current_scancode = 0;
	main_keyboard.char_ready = false;

	memset( main_keyboard.scancode_queue, 0, 255 );
	main_keyboard.scancode_queue_head = 0;
	main_keyboard.scancode_queue_tail = 0;

	// Clear the buffer
	while( inportb(0x64) & 1 ) {
		inportb(0x60);
	}

	interrupt_add_irq_handler( 1, keyboard_interrupt_handler );
}

void keyboard_interrupt_handler( registers **reg ) {
	uint8_t status;
	uint8_t scancode;

	#ifdef KDEBUG_KEYBOARD_INTERRUPT_HANDLER
	log_entry_enter();
	#endif

	status = inportb(0x64);

	if( status & 0x01 ) {
		scancode = inportb(0x60);

		keyboard_add_scancode_to_queue( scancode );
	}

	#ifdef KDEBUG_KEYBOARD_INTERRUPT_HANDLER
	log_entry_exit();
	#endif
}

void keyboard_add_scancode_to_queue( uint8_t code ) {
	if( code == 0 ) {
		return;
	}
	
	main_keyboard.scancode_queue[main_keyboard.scancode_queue_tail] = code;

	main_keyboard.scancode_queue_tail++;

	if( main_keyboard.scancode_queue_tail > 254 ) {
		main_keyboard.scancode_queue_tail = 0;
	}

	if( main_keyboard.is_waiting == true ) {
		task_set_has_data_ready( main_keyboard.waiting_task_id, true );
	}

	//debugf( "added 0x%X\n", code );
}

uint8_t keyboard_get_next_scancode( void ) {
	uint8_t scancode = 0;

	if( main_keyboard.scancode_queue_head == main_keyboard.scancode_queue_tail ) {
		return 0;
	}

	scancode = main_keyboard.scancode_queue[ main_keyboard.scancode_queue_head ];

	main_keyboard.scancode_queue_head++;

	if( main_keyboard.scancode_queue_head > 254 ) {
		main_keyboard.scancode_queue_head = 0;
	}

	return scancode;
}

char keyboard_get_next_char( bool return_special ) {
	uint8_t scancode = 0;
	char ret_val = 0;

	if( main_keyboard.scancode_queue_head == main_keyboard.scancode_queue_tail ) {
		return 0;
	}

	scancode = main_keyboard.scancode_queue[ main_keyboard.scancode_queue_head ];
	
	bool continue_in_queue;

	// Loop until we find a valid character
	do {
		if( scancode == 0xE0 ) {
			main_keyboard.in_E0 = true;
			
			main_keyboard.scancode_queue[main_keyboard.scancode_queue_head] = 0;
			main_keyboard.scancode_queue_head++;

			if( main_keyboard.scancode_queue_head > 254 ) {
				main_keyboard.scancode_queue_head = 0;
			}

			ret_val = keyboard_get_next_char( return_special );
		} else {
			if( main_keyboard.in_E0 ) {
				switch( scancode ) {
					case 0x35:
						ret_val = '/';
						break;
				}

				main_keyboard.in_E0 = false;
			} else {
				// 0x2A Left Sh, 0x36 Right Sh Press
				// 0xAA Left Sh, 0xB6 Right Sh Release
				if( scancode == 0x2A || scancode == 0x36 ) {
					main_keyboard.is_shift = true;
				} else if( scancode == 0xAA || scancode == 0xB6 ) {
					main_keyboard.is_shift = false;
				} else {
					if( scancode < 0x81 ) {
						if( main_keyboard.is_shift ) {
							ret_val = keyboard_map_shift[scancode];
						} else {
							ret_val = keyboard_map[scancode];
						}

						if( return_special ) {
							switch( scancode ) {
								case SCANCODE_ESC:
								case SCANCODE_F1:
								case SCANCODE_F2:
									ret_val = scancode;
									break;
								default:
									break;
							}
						}
					}
				}
			}
		}

		// if we find a character, end
		if( ret_val != 0 ) {
			continue_in_queue = false;

			main_keyboard.scancode_queue[main_keyboard.scancode_queue_head] = 0;

			main_keyboard.scancode_queue_head++;

			if( main_keyboard.scancode_queue_head > 254 ) {
				main_keyboard.scancode_queue_head = 0;
			}
		} else {
			// otherwise increrase queue and continue
			main_keyboard.scancode_queue_head++;

			// fail if we've reached the end
			// otherwise process the next scancode
			if( main_keyboard.scancode_queue_head == main_keyboard.scancode_queue_tail ) {
				continue_in_queue = false;
			} else {
				scancode = main_keyboard.scancode_queue[main_keyboard.scancode_queue_head];
			}
		}
	} while( continue_in_queue );


	return ret_val;
}


char keyboard_get_char( void ) {
	return keyboard_get_char_stage_2( false );
}

char keyboard_get_char_or_special( void ) {
	return keyboard_get_char_stage_2( true );
}

char keyboard_get_char_stage_2( bool return_special ) {
	char c = 0;

	do {
		c = keyboard_get_next_char( return_special );
	} while( c == 0 );

	return c;
}

uint8_t keyboard_get_scancode( void ) {
	uint8_t ret_value = 0;

	main_keyboard.is_waiting = true;
	main_keyboard.waiting_task_id = task_get_current_task_id();
	
	do {
		ret_value = keyboard_get_next_scancode();

		// Yield if we don't have a scancode available
		if( ret_value == 0 ) {
			task_set_task_status( task_get_current_task_id(), TASK_STATUS_WAIT );
			syscall( SYSCALL_SCHED_YIELD, 0, NULL );
		}
	} while( ret_value == 0 );

	task_set_has_data_ready( main_keyboard.waiting_task_id, false );
	main_keyboard.is_waiting = false;
	main_keyboard.waiting_task_id = 0;

	return ret_value;
}

char keyboard_scancode_to_char( uint8_t scancode ) {
	if( scancode < 0x81 ) {
		return keyboard_map[ scancode ];
	}

	return 0;
}