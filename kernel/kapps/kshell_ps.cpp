#include <kernel_common.h>
#include <kshell.h>
#include <task.h>

KSHELL_COMMAND( ps, kshell_app_ps_main )

#define ALWAYS_SEND_TO_DEBUG true
#define COL_SIZE 12

char *status_to_string( int status );

int kshell_app_ps_main( int argc, char *argv[] ) {
	kernel_process_data *process_data = task_get_kernel_process_data();

	printf( "    ID Name         Status       RIP\n" );

	for( int i = 0; i < TASKS_MAX; i++ ) {
		if( process_data->tasks[i] == NULL ) {
			continue;
		}
		
		printf( "%6d %-12s %-12s 0x%016llx\n", i, process_data->tasks[i]->display_name, status_to_string( process_data->tasks[i]->status), process_data->task_contexts[i].rip );
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