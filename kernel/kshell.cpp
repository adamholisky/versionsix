#include <kernel_common.h>
#include <serial.h>
#include <kshell.h>
#include <keyboard.h>
#include <net/arp.h>
#include <kmemory.h>

extern void do_test_send( void );

KShell *main_shell;

KShell::KShell( void ) {
	for( int i = 0; i < KSHELL_MAX_HISTORY; i++ ) {
		lines[i] = (char *)kmalloc( KSHELL_MAX_LINESIZE );
	}

	current_line = (char *)kmalloc( KSHELL_MAX_LINESIZE );
}

void KShell::run( void ) { 
	keep_going = true;

	main_loop();
}

void KShell::stop( void ) {
	keep_going = false;
}

void KShell::main_loop( void ) {
	while( keep_going ) {
		uint8_t scancode = 0;
		char c = 0;
		bool get_next_key = true;

		memset( current_line, 0, KSHELL_MAX_LINESIZE );
		printf( "Version VI: " );

		do {
			scancode = keyboard_get_scancode();
			c = keyboard_scancode_to_char( scancode );	// this checks for scancode under 0x81, otherwise returns 0
			
			if( c != 0 ) {
				if( c == '\n' ) {
					get_next_key = false;
				} else {
					printf( "%c", c );

					current_line[line_index] = c;
					line_index++;

					if( line_index > KSHELL_MAX_LINESIZE - 1 ) {
						get_next_key = false;
					}
				}
			} else {
				get_next_key = handle_special_keypress( scancode );
			}
		} while( get_next_key );
		
		printf( "\n" );

		if( strcmp(current_line, "q") == 0 ) {
			keep_going = false;
		} else if( strcmp(current_line, "a") ) {
			uint8_t dest[] = {10,0,2,2};

			arp_send( (uint8_t *)&dest );
		}
	}
}

/**
 * @brief Handles non-printable keys (excluding bs and ret/ent)
 * 
 * @param scancode 
 * @return true continue with current line
 * @return false start over at a new line 
 */
bool KShell::handle_special_keypress( uint8_t scancode ) {
	switch( scancode ) {
		case SCANCODE_ESC:
			keep_going = false;
			return false;
		case SCANCODE_F1:
			current_line[line_index] = 'a';
			printf( "a\n" );
			return false;
		default:
			return true;
	}
}

void kshell_initalize( void ) {
	main_shell = new KShell();

	main_shell->run();
}