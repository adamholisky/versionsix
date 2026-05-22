#include <kernel_common.h>
#include <devices/serial.h>
#include <process.h>
#include <lib/list.h>
#include <keyboard.h>
#include <stdlib.h>
#include <stacktrace.h>

#include <zydis.h>

bool keep_running;

#define KBUGS_MAX_LINE_SIZE 256
#define KBUGS_MAX_HISTORY 25
#define KBUGS_MAX_ARGS 10

bool keep_running;
bool keep_going;
char kbugs_lines[KBUGS_MAX_HISTORY][KBUGS_MAX_LINE_SIZE];
char kbugs_current_line[KBUGS_MAX_LINE_SIZE];
uint8_t kbugs_line_index;

void *kbugs_data_crash_addr;
registers *kbugs_data_crash_context;
extern kernel_proc_data global_proc_data;

char kbugs_get_c( void );

int kbugs_dm( int argc, char **argv );
int kbugs_sm( int argc, char **argv );
int kbugs_q( int argc, char **argv );
int kbugs_shutdown( int argc, char **argv );
int kbugs_ps( int argc, char **argv );
int kbugs_mem( int argc, char **argv );
int kbugs_kill( int argc, char **argv );
int kbugs_da( int argc, char **argv );
void kbugs_crash_report( void );
void kbugs_cleanup_before_exit( void );

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

	if( kbugs_data_crash_context != NULL ) {
		kbugs_crash_report();
	}

	do {
		uint8_t scancode = 0;
		char c = 0;
		bool get_next_key = true;
		kbugs_line_index = 0;
		bool do_extra_newline = true;

		memset( kbugs_current_line, 0, KBUGS_MAX_LINE_SIZE );
		printf( "kbugs > " );

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
		if( strcmp( argv_builder[0], "da" ) == 0 ) { cmd_to_run = kbugs_da; }
		if( strcmp( argv_builder[0], "dm" ) == 0 ) { cmd_to_run = kbugs_dm; }
		if( strcmp( argv_builder[0], "sm" ) == 0 ) { cmd_to_run = kbugs_sm; }
		if( strcmp( argv_builder[0], "kill" ) == 0 ) { cmd_to_run = kbugs_kill; }
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

	kbugs_cleanup_before_exit();
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

int kbugs_da( int argc, char **argv ) {
	ZyanUSize offset = 0;
	ZydisDisassembledInstruction instruction;

	ZyanU64 runtime_addr = kbugs_data_crash_addr;

	runtime_addr = runtime_addr - 20;

	while( ZYAN_SUCCESS( ZydisDisassembleIntel(
		ZYDIS_MACHINE_MODE_LONG_64,
		runtime_addr,
		kbugs_data_crash_addr + offset,
		50 - offset,
		&instruction
	))) {
		char star = ' ';
		if( runtime_addr == kbugs_data_crash_addr ) {
			star = '*';
		}

		debugf( "%016llX  %c  %s\n", runtime_addr, star, instruction.text );
		offset += instruction.info.length;
		runtime_addr += instruction.info.length;
	}

	return 0;
}

int kbugs_kill( int argc, char **argv ) {
	if( argc != 2 ) {
		printf( "kill syntax: kill <pid>\n" );
		return 0;
	}

	int pid = atol( argv[1] );

	printf( "Killing pid %d\n", pid );

	// This can probably be done in syscall exit somehow
	process_data *p = process_get_data_from_pid(pid);
	p->status = PROCESS_STATUS_WAITING_FOR_DEATH;
	p->exit_code = -1;

	process_data *p_parent = process_get_data_from_pid( p->pid_parent );
	if( p_parent != NULL && p_parent->wait_for_pid == pid ) {
		p_parent->status = PROCESS_STATUS_INACTIVE;
		p_parent->wait_for_pid = 0;
		process_set_next_up( p_parent );
	}

	kbugs_cleanup_before_exit();

	k_yield_in_int();

	return 0;
}

int kbugs_ps( int argc, char **argv ) {
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

void kbugs_cleanup_before_exit( void ) {
	kbugs_data_crash_addr = 0;
	kbugs_data_crash_context = 0;
}

#define kb_out(...) debugf_raw(__VA_ARGS__)

void kbugs_crash_report( void ) {
	process_data *p = process_get_current();
	registers *reg = kbugs_data_crash_context;

	       //================================================================================
	kb_out( "================================================================================\n" );
	kb_out( "Kbugs Crash Report\n" );
	kb_out( "\n" );
	kb_out( "Exception %d: %s \n", reg->interrupt_no, intel_exceptions[reg->interrupt_no] );
	kb_out( "Exec: %s    pid: %d\n", p->path, p->pid );
	kb_out( "rip:  0x%016llX (%s)\n", reg->rip, kernel_symbols_get_function_name_at(reg->rip) );
	kb_out( "rax:  0x%016llX  rbx:  0x%016llX  rcx:  0x%016llX\n", reg->rax, reg->rbx, reg->rcx );
	kb_out( "rdx:  0x%016llX  rsi:  0x%016llX  rdi:  0x%016llX\n", reg->rdx, reg->rsi, reg->rdi );
	kb_out( "rsp:  0x%016llX  rbp:  0x%016llX  cr0:  0x%016llX \n", reg->rsp, reg->rbp, get_cr0() );
	kb_out( "cr2:  0x%016llX  cr3:  0x%016llX  cr4:  0x%016llX\n", get_cr2(), get_cr3(), get_cr4() );
	kb_out( "cs:   0x%04X  num:  0x%08X  err:  0x%08X  flag: 0x%08X\n", reg->cs, reg->interrupt_no, reg->error_no, reg->rflags);
	kb_out( "\n" );
	kb_out( "Stack Trace:\n" );
	
	stacktrace_out_for_rbp( reg->rbp, false, true, 4 );

	kb_out( "\n" );
	kb_out( "Disassembled instruction at pc:\n" );

	ZyanUSize offset = 0;
	ZydisDisassembledInstruction instruction;

	ZyanU64 runtime_addr = kbugs_data_crash_addr;

	runtime_addr = runtime_addr - 20;

	while( ZYAN_SUCCESS( ZydisDisassembleIntel(
		ZYDIS_MACHINE_MODE_LONG_64,
		runtime_addr,
		kbugs_data_crash_addr + offset,
		50 - offset,
		&instruction
	))) {
		char star = ' ';
		if( runtime_addr == kbugs_data_crash_addr ) {
			star = '*';
		}

		kb_out( "0x%016llX  %c  %s\n", runtime_addr, star, instruction.text );
		offset += instruction.info.length;
		runtime_addr += instruction.info.length;
	}

	kb_out( "\n" );
	kb_out( "Process Data:                                   \n" );
	kb_out( "Parent pid: %d    wait_for_pid: %d    type: %d    first_run: %d\n", p->pid_parent, p->wait_for_pid, p->type, p->first_run );
	kb_out( "Stack kvirt: 0x%016llX    virt: 0x%016llX    phys: 0x%016llx\n", p->proc_stack_kvirt, p->proc_stack_virt, p->proc_stack_phys );
	kb_out( "Text section count: %d    virt_start: 0x%016llX\n", p->text_section_count, p->text_secton_virt_start );
	for( int i = 0; i < p->text_section_count; i++ ) {
		kb_out( "    [%d] kvirt: 0x%016llX    virt: 0x%016llX    phys: 0x%016llX\n", i, p->text_sections[i].kern_virt, p->text_sections[i].virt, p->text_sections[i].phys );
	}
	kb_out( "Data section count: %d    virt_start: 0x%016llX\n", p->data_section_count, p->data_section_virt_start );
	for( int i = 0; i < p->data_section_count; i++ ) {
		kb_out( "    [%d] kvirt: 0x%016llX    virt: 0x%016llX    phys: 0x%016llX\n", i, p->data_sections[i].kern_virt, p->data_sections[i].virt, p->data_sections[i].phys );
	}

	kb_out( "\n" );
	kb_out( "Processes running:\n" );
	avs_node *n = global_proc_data.process_list->head;
	for( int i = 0; i < global_proc_data.process_list->size; i ++ )  {
		process_data *pl = n->data;

		kb_out( "ID=%d    Name=%s    Status=%s (%d)\n", pl->pid, pl->name, process_status_text[pl->status], pl->status );
		
		n = n->next;
	}



	kb_out( "================================================================================\n" );
}