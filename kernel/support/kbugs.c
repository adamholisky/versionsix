#include <kernel_common.h>
#include <serial.h>
#include <process.h>
#include <lib/list.h>

bool keep_running;

#define KBUGS_MAX_LINE_SIZE 256
#define KBUGS_MAX_HISTORY 25
#define KBUGS_MAX_ARGS 10

bool keep_running;
bool keep_going;
char kbugs_lines[KBUGS_MAX_HISTORY][KBUGS_MAX_LINE_SIZE];
char kbugs_current_line[KBUGS_MAX_LINE_SIZE];
uint8_t kbugs_line_index;

char kbugs_get_c( void );

int kbugs_dm( int argc, char **argv );
int kbugs_sm( int argc, char **argv );
int kbugs_q( int argc, char **argv );
int kbugs_shutdown( int argc, char **argv );
int kbugs_ps( int argc, char **argv );
int kbugs_mem( int argc, char **argv );

typedef int (*kbugs_cmd)( int num_args, char *arg_list[] );
#define kbugs_cmd_check_and_run(x,fn) if( strcmp( argv_builder[0], ##x ) == 0 ) { fn## ( num_args, argv_builder ); }

char kbugs_get_c( void ) {
	char c = ' ';
	int scancode = 0;

	#ifdef STDIO_SERIAL_3
		c = serial_read_port( COM3 );
	#else
		scancode = keyboard_get_scancode();
		c = keyboard_scancode_to_char( scancode );
	#endif

	return c;
}

void kbugs_main( void ) {
	keep_running = true;

	printf( "Welcome to kbugs.\n" );

	do {
		uint8_t scancode = 0;
		char c = 0;
		bool get_next_key = true;
		kbugs_line_index = 0;
		bool do_extra_newline = true;

		memset( kbugs_current_line, 0, KBUGS_MAX_LINE_SIZE );
		printf( "> " );

		/* Step 1: Get the line, put it into current_line */
		do {
			//main_console_set_cursor_visiblity( false );
			//scancode = keyboard_get_scancode();
			//c = keyboard_scancode_to_char( scancode );	// this checks for scancode under 0x81, otherwise returns 0
			
			c = kbugs_get_c();

			//debugf( "c: %c (%d)\n", c, (int)c );

			if( c != 0 ) {
				if( c == '\n' || c == 13 ) {
					get_next_key = false;
					do_extra_newline = false;
					printf( "\n" );
				} else if( c == '\b' || c == 127 ) {
					if( kbugs_line_index != 0 ) {
						kbugs_line_index--;
						kbugs_current_line[kbugs_line_index] = 0;

						printf( "\b" );
					}					
				} else {
					printf( "%c", c );

					kbugs_current_line[kbugs_line_index] = c;
					kbugs_line_index++;

					if( kbugs_line_index > KBUGS_MAX_LINE_SIZE - 1 ) {
						get_next_key = false;
					}
				}
			} else {
				//get_next_key = kshell_handle_special_keypress( scancode );
			}
		} while( get_next_key );
		
		//main_console_set_cursor_visiblity( false );

		// don't do a double newline if enter key was pressed
		if( do_extra_newline == true ) {
			printf( "\n" );
		}

		/* Step 1.a: Prevent a blank line */
		if( strcmp( kbugs_current_line, "" ) == 0 ) {
			continue;
		}

		/* Step 2: Split it up into arguments, create argc and argv */

		bool keep_processing_line = true;
		char args[KBUGS_MAX_ARGS][KBUGS_MAX_LINE_SIZE];
		char *argv_builder[KBUGS_MAX_ARGS];
		char *char_to_process = kbugs_current_line;
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

		kbugs_cmd cmd_to_run = NULL;
		
		if( strcmp( argv_builder[0], "sd" ) == 0 ) { cmd_to_run = kbugs_shutdown; }
		if( strcmp( argv_builder[0], "ps" ) == 0 ) { cmd_to_run = kbugs_ps; }
		if( strcmp( argv_builder[0], "dm" ) == 0 ) { cmd_to_run = kbugs_dm; }
		if( strcmp( argv_builder[0], "sm" ) == 0 ) { cmd_to_run = kbugs_sm; }
		if( strcmp( argv_builder[0], "q" ) == 0 ) { cmd_to_run = kbugs_q; }
		if( strcmp( argv_builder[0], "shutdown" ) == 0 ) { cmd_to_run = kbugs_shutdown; }


		int cmd_return_value = 0;

		if( cmd_to_run != NULL ) {
			cmd_return_value = cmd_to_run( num_args, argv_builder );
		} else {
			printf( "%s: command not found\n", argv_builder[0] );
		}

		/* Step 6: For now display any non-zero return code */

		if( cmd_return_value != 0 ) {
			printf( "%s: Error %d\n", argv_builder[0], cmd_return_value );
		}

	} while( keep_running );
}

int kbugs_dm( int argc, char **argv ) {
	printf( "In kbugs dm\n" );

	return 0;
}

int kbugs_sm( int argc, char **argv ) {
	printf( "in kbugs sm\n" );

	return 0;
}

int kbugs_q( int argc, char **argv ) {
	keep_running = false;
	return 0;
}

int kbugs_shutdown( int argc, char **argv ) {
	do_immediate_shutdown();

	return 0;
}

int kbugs_ps( int argc, char **argv ) {
	extern kernel_proc_data global_proc_data;

	printf( "Current pid: %d\n", global_proc_data.current_process->pid );

	printf( "Proc list\n====================\n" );

	avs_node *n = global_proc_data.process_list->head;
	for( int i = 0; i < global_proc_data.process_list->size; i ++ )  {
		process_data *p = n->data;

		printf( "ID=%d    Name=%s    Status=%d    more=0x%016llX\n", p->pid, p->name, p->status, p );
		
		n = n->next;
	}

	return 0;
}

int kbugs_mem( int argc, char **argv ) {

}