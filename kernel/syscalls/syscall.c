#include <kernel_common.h>
#include <interrupt.h>
#include <syscall.h>
#include <process.h>

extern pid_t fork_syscall_handler( registers **context );

void syscall_initalize( void ) {
	interrupt_add_irq_handler( 0xFE - 0x20, syscall_handler );
}

uint64_t syscall( uint64_t call_num, uint8_t num_args, syscall_args *args ) {
	uint64_t ret = 0;
	int arg3 = 0;
	
	switch( num_args ) {
		case 6:
		case 5:
		case 4:
		case 3:
			__asm__	__volatile__ ( 
				"movq %1, %%rax \n"
				"movq %2, %%rdi \n"
				"movq %3, %%rsi \n"
				"movq %4, %%rdx \n"
				"int %5 \n"
				"movq %%rax, %0"
				:"=r"(ret)
				:"r"(call_num), "m"(args->arg_1), "m"(args->arg_2), "m"(args->arg_3), "i"(0xFE)
				:"%rax", "%rdi", "%rsi", "%rdx" 
			);
			break;
		case 2:
			__asm__	__volatile__ ( 
				"movq %1, %%rax \n"
				"movq %2, %%rdi \n"
				"movq %3, %%rsi \n"
				"int %4 \n"
				"movq %%rax, %0"
				:"=r"(ret)
				:"r"(call_num), "m"(args->arg_1), "m"(args->arg_2), "i"(0xFE)
				:"%rax", "%rdi", "%rsi" 
			);
			break;
		case 1:
			__asm__	__volatile__ ( 
				"movq %1, %%rax \n"
				"movq %2, %%rdi \n"
				"int %3 \n"
				"movq %%rax, %0"
				:"=r"(ret)
				:"r"(call_num), "m"(args->arg_1), "i"(0xFE)
				:"%rax", "%rdi" 
			);
			break;
		case 0:
			__asm__	__volatile__ ( 
				"movq %1, %%rax \n"
				"int %2 \n"
				"movq %%rax, %0"
				:"=r"(ret)
				:"r"(call_num), "i"(0xFE)
				:"%rax" 
			);
			break;
		default:
			debugf( "Syscall called with more than six args.\n" );
	}

	return ret;
}

char *syscall_log_msg_first = "In syscall. num=";

void syscall_handler( registers **_context ) {
	registers *context = *_context;

	switch( context->rax ) {
		case SYSCALL_FORK:
			fork_syscall_handler( _context );
			break;
		case SYSCALL_SCHED_YIELD:
			process_sched_yield( _context );
			break;
		case SYSCALL_EXECVE:
			klog( LOG_INFO, "%s%d path=\"%s\"  argv=0x%016llX evnp=0x%016llX", syscall_log_msg_first, context->rax, context->rdi, context->rsi, context->rdx );
			execve_syscall_handler( _context, context->rdi, context->rsi, context->rdx );
			break;
		case SYSCALL_EXIT:
			exit_syscall_handler( _context, context->rdi );
			break;
		case SYSCALL_WAIT:
			wait_syscall_handler( _context, context->rdi );
			break;
		default:
			debugf( "Unhandled syscall number: %d\n", context->rax );
	}

	// Because syscall handler is special
	outportb( PIC_SECONDARY_COMMAND, PIC_EOI );
	outportb( PIC_PRIMARY_COMMAND, PIC_EOI );
}