#include <kernel_common.h>
#include <kshell_app.h>
#include <task.h>
#include <stacktrace.h>
#include <ksymbols.h>

KSHELL_COMMAND( ps, kshell_app_ps_main )

#define ALWAYS_SEND_TO_DEBUG true
#define COL_SIZE 12

char *status_to_string( int status );

int kshell_app_ps_main( int argc, char *argv[] ) {
	kernel_process_data *process_data = task_get_kernel_process_data();
	task *task_next = process_data->tasks;

	bool show_i = false;
	bool show_bt = false;
	bool show_help = false;
	bool show_main = true;

	if( argc > 1 ) {
		for( int i = 1; i < argc; i++ ) {
			if( strcmp( argv[i], "-i" ) == 0 ) {
				show_i = true;
			}

			if( strcmp( argv[i], "-bt" ) == 0 ) {
				show_bt = true;
			}

			if( strcmp( argv[i], "-h" ) == 0 ) {
				show_help = true;
			}
		}
	}

	if( show_help ) {
		printf( "-i        Show i count\n" );
		printf( "-bt       Show backtraces for each process\n" );
		show_main = false;
	}
	
	if( show_main ) {
		printf( "    ID Name         Status       RIP\n" );

		do {
			uint64_t rip = task_next->task_context.rip;

			if( task_next->status == TASK_STATUS_ACTIVE ) {
				__asm__	__volatile__ ( "lea 0(%%rip), %0" : "=r"(rip) );
			}

			printf( "%6d %-12s %-12s 0x%016llx %s\n", 
				task_next->id, 
				task_next->display_name, 
				status_to_string( task_next->status), 
				rip, 
				kernel_symbols_get_function_name_at( rip ) 
				);

			if( show_bt ) {
				uint64_t rbp = task_next->task_context.rbp;

				if( task_next->status == TASK_STATUS_ACTIVE ) {
					__asm__	__volatile__ ( "movq %%rbp, %0"	: "=r"(rbp) );
				}

				stacktrace_out_for_rbp( rbp, true, false, 33 );
				printf( "\n" );
			}

			task_next = task_next->next;
		} while( task_next != NULL );

		if( show_i ) {
			printf( "\nKernel i count: %d\n", get_i() );
		}
	}
	

	return 0;
}

char *status_to_string( int status ) {
	switch( status ) {
		case TASK_STATUS_READY:
			return "ready";
			break;
		case TASK_STATUS_ACTIVE:
			return "active";
			break;
		case TASK_STATUS_INACTIVE:
			return "inactive";
			break;
		case TASK_STATUS_WAIT:
			return "wait";
			break;
		case TASK_STATUS_DEAD:
			return "dead";
			break;
		default:
			return "unknown";
			break;
	}
}