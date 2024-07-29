#include <kernel_common.h>
#include <serial.h>
#include <kshell.h>
#include <keyboard.h>
#include <net/arp.h>
#include <kmemory.h>
#include <ksymbols.h>
#include <task.h>

kshell_config main_shell;
kshell_command_list kshell_commands;

kshell_env_var_list env_vars;

void kshell_initalize( void ) {
	// Setup first command
	kshell_commands.cmd = kshell_command_create( "hw", (void *)kshell_command_hello_world );
	kshell_commands.next = NULL;

	// Find all symbols that start wtih "kshell_app_add_command" and call them each
	symbol_collection *ksym = get_ksyms_object();
	symbol *symbol_array = symbols_get_symbol_array( ksym );
	uint64_t max_symbols = symbols_get_total_symbols( ksym );

	for( int i = 0; i < max_symbols; i++ ) {
		if( strncmp( symbol_array[i].name, "kshell_app_add_command_", sizeof("kshell_app_add_command_") - 1 ) == 0 ) {
			debugf( "found: %s\n", symbol_array[i].name );
			void (*func)(void) = (void(*)(void))symbol_array[i].addr;
			func();
		}
	}

	for( int i = 0; i < KSHELL_MAX_HISTORY; i++ ) {
		main_shell.lines[i] = (char *)kmalloc( KSHELL_MAX_LINESIZE );
	}

	main_shell.current_line = (char *)kmalloc( KSHELL_MAX_LINESIZE );

	env_vars.num_vars = 1;
	env_vars.top = kmalloc( sizeof(kshell_env_var) );
	strcpy( env_vars.top->name, "WD" );
	strcpy( env_vars.top->value, "/" );
	env_vars.top->next = NULL;

	kshell_set_env_var( "PATH", "/bin" );
	kshell_set_env_var( "HOST", "qemu1" );
	kshell_set_env_var( "USER", "root" );

	kshell_run();
}

void kshell_run( void ) { 
	main_shell.keep_going = true;

	printf( "%c%c%c\n", 0xDA, 0xC4, 0xBF );
	printf( "%c %c\n", 0x7C, 0x7C );

	kshell_main_loop();

	do_immediate_shutdown();
}

void kshell_stop( void ) {
	main_shell.keep_going = false;

	#ifdef VIOS_ENABLE_PROFILING
	symbol_collection *ksyms = get_ksyms_object();
	debugf( "Profiling Counts:\n===================\n" );
	for( int i = 0; i < ksyms->top; i++ ) {
		if( ksyms->symbols[i].count != 0 ) {
			debugf_raw( "%s\t%d\t%ld\n", ksyms->symbols[i].name, ksyms->symbols[i].count, ksyms->symbols[i].time );
		}
	}
	debugf( "====================\n" );
	#endif


	do_immediate_shutdown();
}

void kshell_main_loop( void ) {
	char *user_name = kshell_get_env_var( "USER" );
	char *host = kshell_get_env_var( "HOST" );

	while( main_shell.keep_going ) {
		uint8_t scancode = 0;
		char c = 0;
		bool get_next_key = true;
		main_shell.line_index = 0;
		bool do_extra_newline = true;

		memset( main_shell.current_line, 0, KSHELL_MAX_LINESIZE );
		printf( "[VVI %s@%s %s]: ", user_name, host, kshell_get_env_var("WD") );

		/* Step 1: Get the line, put it into current_line */
		do {
			main_console_set_cursor_visiblity( false );
			scancode = keyboard_get_scancode();
			c = keyboard_scancode_to_char( scancode );	// this checks for scancode under 0x81, otherwise returns 0
			
			if( c != 0 ) {
				if( c == '\n' ) {
					get_next_key = false;
					do_extra_newline = false;
					printf( "\n" );
				} else if( c == '\b' ) {
					if( main_shell.line_index != 0 ) {
						main_shell.line_index--;
						main_shell.current_line[main_shell.line_index] = 0;

						printf( "\b" );
					}					
				} else {
					printf( "%c", c );

					main_shell.current_line[main_shell.line_index] = c;
					main_shell.line_index++;

					if( main_shell.line_index > KSHELL_MAX_LINESIZE - 1 ) {
						get_next_key = false;
					}
				}
			} else {
				get_next_key = kshell_handle_special_keypress( scancode );
			}
		} while( get_next_key );
		
		main_console_set_cursor_visiblity( false );

		// don't do a double newline if enter key was pressed
		if( do_extra_newline == true ) {
			printf( "\n" );
		}

		/* Step 1.a: Prevent a blank line */
		if( strcmp( main_shell.current_line, "" ) == 0 ) {
			continue;
		}

		/* Step 2: Split it up into arguments, create argc and argv */

		bool keep_processing_line = true;
		char args[KSHELL_MAX_ARGS][KSHELL_MAX_LINESIZE];
		char *argv_builder[KSHELL_MAX_ARGS];
		char *char_to_process = main_shell.current_line;
		int num_args = 0;
		int i = 0;
		int j = 0;

		do {
			if( *char_to_process != ' ' && *char_to_process != 0 ) {
				args[i][j] = *char_to_process; 
				j++;
			} else {
				if( j != 0 ) {
					num_args++;
				}

				args[i][j] = 0;
				i++;
				j = 0;

				if( i > 3 ) {
					keep_processing_line = false;
				}
			}

			char_to_process++;
		} while( keep_processing_line );

		//debugf( "num_args = %d\n", num_args );
	
		for( int z = 0; z < num_args; z++ ) {
			//debugf( "args[%d] = \"%s\"\n", z, args[z] );

			argv_builder[z] = args[z];
		} 

		/* Step 3: See if we have the command in our special kernel list */

		kshell_command_list *head = &kshell_commands;
		kshell_command *to_run = NULL;

		while( head != NULL ) {
			if( strcmp( argv_builder[0], head->cmd->name ) == 0 ) {
				to_run = head->cmd;
				head = NULL;
			} else {
				head = (kshell_command_list *)head->next;
			}
		}

		/* Step 4: TODO! Query the file system for the command */

		/* Step 5: Run the command, or fail*/

		int cmd_return_value = 0;

		if( to_run != NULL ) {
			cmd_return_value = kshell_command_run( to_run, num_args, argv_builder );
		} else {
			printf( "%s: command not found\n", argv_builder[0] );
		}

		/* Step 6: For now display any non-zero return code */

		if( cmd_return_value != 0 ) {
			printf( "%s: Error %d\n", argv_builder[0], cmd_return_value );
		}

		//syscall( SYSCALL_SCHED_YIELD, 0, NULL );
	}
}

/**
 * @brief Handles non-printable keys (excluding bs and ret/ent)
 * 
 * @param scancode 
 * @return true continue with current line
 * @return false start over at a new line 
 */
bool kshell_handle_special_keypress( uint8_t scancode ) {
	switch( scancode ) {
		case SCANCODE_ESC:
			kshell_stop();
			return false;
		case SCANCODE_F1:
			return true;
		default:
			return true;
	}
}

/**
 * @brief Create a kshell_command and return a pointer to it
 * 
 * @param command_name display name AND command name
 * @param main_function pointer to the function address to run
 * @return kshell_command* pointer to a new kshell_command
 */
kshell_command __attribute__ ((no_instrument_function)) *kshell_command_create( char *command_name, void *main_function ) {
	kshell_command *cmd = (kshell_command *)kmalloc( sizeof( kshell_command ) );
	strcpy( cmd->name, command_name );
	cmd->entry = main_function;
}

/**
 * @brief Create and run a kshell_command
 * 
 * @param cmd Pointer to a kshell_command object to run
 * @param argc number of arguments
 * @param argv pointer array to list of null terminated strings
 * @return int exit code as given by command
 */
#undef DEBUG_KSHELL_COMMAND_RUN
int kshell_command_run( kshell_command *cmd, int argc, char *argv[] ) {
	#ifdef DEBUG_KSHELL_COMMAND_RUN
	log_entry_enter();
	#endif

	kshell_main_func_to_call func = (kshell_main_func_to_call)cmd->entry;

	uint16_t cmd_task_id = task_create( TASK_TYPE_KERNEL_PROCESS, cmd->name, cmd->entry );

	syscall_args exec_args;
	exec_args.arg_1 = cmd_task_id;
	exec_args.arg_2 = argc;
	exec_args.arg_3 = (uint64_t)argv;

	#ifdef DEBUG_KSHELL_COMMAND_RUN
	debugf( "Name: %s\n", cmd->name );
	debugf( "Entry: 0x%016llx\n", cmd->entry );
	debugf( "Task I: %d\n", cmd_task_id );
	#endif

	syscall( SYSCALL_EXEC, 3, &exec_args );

	uint64_t exit_code = task_get_exit_code( cmd_task_id );

	#ifdef DEBUG_KSHELL_COMMAND_RUN
	debugf( "Exit code: %d\n", exit_code );
	log_entry_exit();
	#endif
	return (int)exit_code;
}

int kshell_command_hello_world( int argc, char *argv[] ) {
	printf( "Hello, world!\n" );
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

char *kshell_get_env_var( char *name ) {
	kshell_env_var *head = env_vars.top;

	while( head != NULL ) {
		if( strcmp( head->name, name ) == 0 ) {
			return head->value;
		}

		head = head->next;
	}

	return NULL;
}

void kshell_set_env_var( char *name, char *value ) {
	kshell_env_var *head = env_vars.top;
	kshell_env_var *tail = kmalloc( sizeof(kshell_env_var) );
	strcpy( tail->name,  name );
	strcpy( tail->value, value );
	tail->next = NULL;

	do {
		if( head->next == NULL ) {
			head->next = tail;
			return;
		}

		head = head->next;
	} while( head != NULL );
}