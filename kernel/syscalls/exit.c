#include <stdint.h>
#include <stddef.h>
#include "syscall.h"
#include "file.h"
#include "serial.h"
#include <device.h>
#include <process.h>
#include <debug.h>

kernel_proc_data global_proc_data;

void exit_syscall_handler( registers **context, int return_code ) {
	klog( LOG_INFO, "Exit pid=%d  ret_code=%d", global_proc_data.current_process->pid, return_code );

	global_proc_data.current_process->exit_code = return_code;
	global_proc_data.current_process->status = PROCESS_STATUS_DEAD;

	syscall( SYSCALL_SCHED_YIELD, 0, NULL );
}