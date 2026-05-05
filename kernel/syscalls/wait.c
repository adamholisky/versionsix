#include <stdint.h>
#include <stddef.h>
#include "syscall.h"
#include "file.h"
#include "serial.h"
#include <device.h>
#include <process.h>
#include <debug.h>

kernel_proc_data global_proc_data;

int wait_syscall_handler( registers **context, pid_t pid ) {
	klog( LOG_INFO, "Wait calling_pid=%d  waiting_for=%d", global_proc_data.current_process->pid, pid );

	

	syscall( SYSCALL_SCHED_YIELD, 0, NULL );
}