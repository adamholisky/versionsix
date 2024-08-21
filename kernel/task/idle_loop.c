#include <kernel_common.h>
#include <task.h>

uint64_t i;

void kernel_idle_loop( void ) {
    i = 0;

	do {
		i++; // Keep track of how many times we loop

		bool found_ready = false;
		kernel_process_data *process_data = task_get_kernel_process_data();
		task *head = NULL;
		
		// Loop until we have found a task that's ready to run
		do { 
			head = process_data->tasks;

			 // Loop over tasks, looking for one that's ready to run
			do {
				if( head->has_data_ready == true ) {
					found_ready = true;
					continue;
				} else {
					head = head->next;
				}
			} while( head != NULL && found_ready == false );

		} while( found_ready == false );
		
		// At this point we know there's someone with data ready, so yield until we find it
		syscall( SYSCALL_SCHED_YIELD, 0, NULL );
	} while( 1 );
}

uint64_t get_i( void ) {
	return i;
}