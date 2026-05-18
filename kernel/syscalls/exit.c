#include <stdint.h>
#include <stddef.h>
#include "syscall.h"
#include "file.h"
#include "serial.h"
#include <device.h>
#include <process.h>
#include <debug.h>

extern kernel_proc_data global_proc_data;

void exit( int status ) {
	syscall_args args = {
		.arg_1 = status
	};

	syscall( SYSCALL_EXIT, 1, &args );
}

void exit_syscall_handler( registers **context, int return_code ) {
	klog( LOG_INFO, "Exit pid=%d  ret_code=%d", global_proc_data.current_process->pid, return_code );

	// Close out FDs
	for( int i = 0; i < PROCESS_MAX_FDS; i++ ) {
		if( global_proc_data.current_process->file_descriptors[i].in_use == true ) {
			if( i < 3 ) {
				// Ignore stdin, stdout, stderr
				continue;
			}
			
			close_syscall_handler( NULL, i );
		}
	}

	// Handle parent notifications
	process_data *p_parent = process_get_data_from_pid( global_proc_data.current_process->pid_parent );
	if( p_parent != NULL && p_parent->wait_for_pid == global_proc_data.current_process->pid ) {
		p_parent->status = PROCESS_STATUS_INACTIVE;
		p_parent->wait_for_pid = 0;
		process_set_next_up( p_parent );
	}

	// This should be the last thing, incase we get interrupted
	global_proc_data.current_process->exit_code = return_code;
	global_proc_data.current_process->status = PROCESS_STATUS_WAITING_FOR_DEATH;

	klog( LOG_INFO, "Exit syscall now yielding." );

	process_sched_yield( context );
}