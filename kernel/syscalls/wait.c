#include <stdint.h>
#include <stddef.h>
#include "syscall.h"
#include "file.h"
#include "serial.h"
#include <device.h>
#include <process.h>
#include <debug.h>

pid_t wait( pid_t pid ) {
	syscall_args args = {
		.arg_1 = pid
	};

	syscall( SYSCALL_WAIT, 1, &args );
	syscall( SYSCALL_SCHED_YIELD, 0, NULL );
}

extern kernel_proc_data global_proc_data;

int wait_syscall_handler( registers **context, pid_t pid ) {
	klog( LOG_INFO, "Wait calling_pid=%d  waiting_for=%d", global_proc_data.current_process->pid, pid );

	global_proc_data.current_process->status = PROCESS_STATUS_WAITING;
	global_proc_data.current_process->wait_for_pid = pid;
}