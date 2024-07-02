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
	int scancode;

	#ifdef KDEBUG_KEYBOARD_INTERRUPT_HANDLER
	log_entry_enter();
	#endif

	debugf( "called\n" );

	status = inportb(0x64);

	if( status & 0x01 ) {
		scancode = inportb(0x60);

		if( scancode == 42 || scancode == 54 ) {
			is_shift = true;
		} else if( scancode == -86 || scancode == -74 )	{
			is_shift = false;
		}
		
		if( scancode > 0 && scancode < 0x81) {
			if( is_shift ) {
				debugf( "shift %d\n", scancode );
				add_scancode_to_queue( keyboard_map_shift[scancode] );
			} else {
				debugf( "noshift 0x%X\n", scancode );
				add_scancode_to_queue( keyboard_map[scancode] );
			}
		}
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

	debugf( "added %d %c\n", code, code );

	char_ready = true;
}

uint8_t Keyboard::get_next_scancode( void ) {
    return 0;
}

char Keyboard::get_next_char( void ) {
    char ret_val = scancode_queue[scancode_queue_head];

    scancode_queue[scancode_queue_head] = 0;

    scancode_queue_head++;

    if( scancode_queue_head > 254 ) {
        scancode_queue_head = 0;
    }

	if( scancode_queue_head == scancode_queue_tail ) {
		char_ready = false;
	}

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
	while( main_keyboard->char_ready == false ) {
		;
	}

	return main_keyboard->get_next_char();
}