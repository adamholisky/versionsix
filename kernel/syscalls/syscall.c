#include <kernel_common.h>
#include <interrupt.h>
#include <syscall.h>
#include <task.h>

void syscall_initalize( void ) {
    interrupt_add_irq_handler( 0xFE - 0x20, syscall_handler );
}

uint64_t syscall( uint64_t call_num, uint8_t num_args, syscall_args *args ) {
    uint64_t ret = 0;
    int arg3 = 0;

	// TODO: WHY DO WE NEED THIS?!?!?
    if( num_args > 0 ) {
        arg3 = args->arg_3;
    }
	
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
				:"r"(call_num), "m"(args->arg_1), "m"(args->arg_2), "m"(arg3), "i"(0xFE)
				:"%rax" 
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
				:"%eax" 
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
				:"%rax" 
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

void syscall_handler( registers **_context ) {
	registers *context = *_context;
    switch( context->rax ) {
        case SYSCALL_SCHED_YIELD:
            task_sched_yield( _context );
            break;
		case SYSCALL_EXEC:
			task_exec( (uint64_t *)context->rax, context->rdi, (uint64_t *)context->rsi );
			break;
        default:
            debugf( "Unhandled syscall number: %d\n", context->rax );
    }
}