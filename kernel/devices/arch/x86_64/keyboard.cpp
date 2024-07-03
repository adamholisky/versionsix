#include <kernel_common.h>
#include <interrupt.h>
#include <keyboard.h>

Keyboard *main_keyboard;

unsigned char keyboard_map[128] = {
	0, 27, '1', '2', '3', '4', '5', '6', '7', '8',	  /* 9 */
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
	0, 27, '!', '@', '#', '$', '%', '^', '&', '*',	  /* 9 */
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


Keyboard::Keyboard( void ) {
    is_waiting = false;
    is_shift = false;
	in_E0 = false;
    current_scancode = 0;
	char_ready = false;

	memset( scancode_queue, 0, 255 );
    scancode_queue_head = 0;
	scancode_queue_tail = 0;

    // Clear the buffer
	while( inportb(0x64) & 1 ) {
		inportb(0x60);
	}
}

void Keyboard::interrupt_handler( void ) {
    uint8_t status;
	uint8_t scancode;

	#ifdef KDEBUG_KEYBOARD_INTERRUPT_HANDLER
	log_entry_enter();
	#endif

	status = inportb(0x64);

	if( status & 0x01 ) {
		scancode = inportb(0x60);

		add_scancode_to_queue( scancode );
	}

	#ifdef KDEBUG_KEYBOARD_INTERRUPT_HANDLER
	log_entry_exit();
	#endif
}

void Keyboard::add_scancode_to_queue( uint8_t code ) {
	if( code == 0 ) {
		return;
	}
    
	scancode_queue[scancode_queue_tail] = code;

	scancode_queue_tail++;

	if( scancode_queue_tail > 254 ) {
		scancode_queue_tail = 0;
	}

	//debugf( "added 0x%X\n", code );
}

uint8_t Keyboard::get_next_scancode( void ) {
	uint8_t scancode = 0;

	if( scancode_queue_head == scancode_queue_tail ) {
		return 0;
	}

	scancode = scancode_queue[ scancode_queue_head ];

	scancode_queue_head++;

	if( scancode_queue_head > 254 ) {
		scancode_queue_head = 0;
	}

    return scancode;
}

char Keyboard::get_next_char( bool return_special ) {
	uint8_t scancode = 0;
	char ret_val = 0;

	if( scancode_queue_head == scancode_queue_tail ) {
		return 0;
	}

	scancode = scancode_queue[ scancode_queue_head ];
	
	bool continue_in_queue;

	// Loop until we find a valid character
	do {
		if( scancode == 0xE0 ) {
			in_E0 = true;
			
			scancode_queue[scancode_queue_head] = 0;
			scancode_queue_head++;

			if( scancode_queue_head > 254 ) {
				scancode_queue_head = 0;
			}

			ret_val = get_next_char( return_special );
		} else {
			if( in_E0 ) {
				switch( scancode ) {
					case 0x35:
						ret_val = '/';
						break;
				}

				in_E0 = false;
			} else {
				// 0x2A Left Sh, 0x36 Right Sh Press
				// 0xAA Left Sh, 0xB6 Right Sh Release
				if( scancode == 0x2A || scancode == 0x36 ) {
					is_shift = true;
				} else if( scancode == 0xAA || scancode == 0xB6 ) {
					is_shift = false;
				} else {
					if( scancode < 0x81 ) {
						if( is_shift ) {
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

			scancode_queue[scancode_queue_head] = 0;

			scancode_queue_head++;

			if( scancode_queue_head > 254 ) {
				scancode_queue_head = 0;
			}
		} else {
			// otherwise increrase queue and continue
			scancode_queue_head++;

			// fail if we've reached the end
			// otherwise process the next scancode
			if( scancode_queue_head == scancode_queue_tail ) {
				continue_in_queue = false;
			} else {
				scancode = scancode_queue[scancode_queue_head];
			}
		}
	} while( continue_in_queue );


    return ret_val;
}

void keyboard_initalize( void ) {
    main_keyboard = new Keyboard();

	interrupt_add_irq_handler( 1, keyboard_interrupt_handler );
}

void keyboard_interrupt_handler( registers **reg ) {
    main_keyboard->interrupt_handler();
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
		c = main_keyboard->get_next_char( return_special );
	} while( c == 0 );

	return c;
}

char scancode_to_char( uint8_t scancode ) {
	if( scancode < 0x81 ) {
		return keyboard_map[ scancode ];
	}

	return 0;
}