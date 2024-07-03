#include <kernel_common.h>
#include <serial.h>
#include <kshell.h>
#include <keyboard.h>
#include <net/arp.h>
#include <kmemory.h>
#include <ksymbols.h>

extern void do_test_send( void );

KShell *main_shell;

kshell_command_list kshell_commands;

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
		line_index = 0;

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

		kshell_command_list *head = &kshell_commands;
		kshell_command *to_run = NULL;

		while( head != NULL ) {
			if( strcmp( current_line, head->cmd->name ) == 0 ) {
				to_run = head->cmd;
				head = NULL;
			} else {
				head = (kshell_command_list *)head->next;
			}
		}

		if( to_run != NULL ) {
			kshell_command_run( to_run, 0, NULL );
		} else {
			printf( "Command not found.\n" );
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

kshell_command *kshell_command_create( char *command_name, void *main_function ) {
	kshell_command *cmd = (kshell_command *)kmalloc( sizeof( kshell_command ) );
	strcpy( cmd->name, command_name );
	cmd->entry = main_function;
}

int kshell_command_run( kshell_command *cmd, int argc, char *argv[] ) {
	kshell_main_func_to_call func = (kshell_main_func_to_call)cmd->entry;

	return func( 0, NULL );
}

int kshell_command_hello_world( int argc, char *argv[] ) {
	printf( "Hello, world!\n" );
}

void kshell_initalize( void ) {
	main_shell = new KShell();
	
	// Setup first command
	kshell_commands.cmd = kshell_command_create( "hw", (void *)kshell_command_hello_world );
	kshell_commands.next = NULL;

	// Find all symbols that start wtih "kshell_app_add_command" and call them each
	KernelSymbols *ksym = get_ksyms_object();
	KernelSymbol *symbol_array = ksym->get_symbol_array();
	uint64_t max_symbols = ksym->get_total_symbols();

	for( int i = 0; i < max_symbols; i++ ) {
		if( strncmp( symbol_array[i].name, "kshell_app_add_command_", sizeof("kshell_app_add_command_") - 1 ) == 0 ) {
			void (*func)(void) = (void(*)(void))symbol_array[i].addr;
			func();
		}
	}

	main_shell->run();
}

void kshell_add_command( char *command_name, void *main_function ) {
	kshell_command_list *head = &kshell_commands;
	kshell_command_list *entry = NULL;

	do {
		if( head->next == NULL ) {
			head->next = (kshell_command_list *)kmalloc( sizeof(kshell_command_list) );
			entry = (kshell_command_list *)head->next;
		} else {
			head = (kshell_command_list *)head->next;
		}
	} while( entry == NULL );

	if( entry == NULL ) {
		debugf( "Entry is null. Bailing.\n" );
		return;
	}

	entry->cmd = kshell_command_create( command_name, main_function );
	entry->next = NULL;
}