#include <kernel_common.h>
#include <syscall.h>
#include <process.h>
#include <page.h>
#include <vfs.h>

extern kernel_proc_data global_proc_data;

int clone( void (*start)( void *), int flags, void *args ) {
	syscall_args _args = {
		.arg_1 = start,
		.arg_2 = flags,
		.arg_3 = args
	};

	debugf( "Doing clone syscall\n" );
	klog( LOG_DEBUG, "Doign clone syscall" );

	return syscall( SYSCALL_CLONE, 3, &_args );
}

int clone_syscall_handler( registers **context, void (*start)( void *), void *stack, int flags, void *args ) {
	// new stack, but shared memory + code

	debugf( "In clone syscall start= 0x%016llX  stack=0x%016llX  flags=%d  args=0x%016llX\n", start, stack, flags, args );
	klog( LOG_DEBUG, "In clone." );

	process_data *p_parent = process_get_current();
	pid_t pid_parent = p_parent->pid;

	pid_t pid_child = process_get_new_process();

	process_data *p_child = process_get_data_from_pid( pid_child );

	if( p_child == NULL ) {
		debugf( "p_child is null.\n" );
		klog( LOG_PANIC, "p_child is null." );
		do_immediate_shutdown();
	}

	p_child->argc = p_parent->argc;
	p_child->argv = p_parent->argv;
	p_child->binary_format_data = p_parent->binary_format_data;
	p_child->exec_size = p_parent->exec_size;
	p_child->has_own_addr_space = false;
	p_child->pid_parent = p_parent->pid;
	p_child->first_run = true;
	p_child->type = PROCESS_TYPE_THREAD;
	
	strcpy( p_child->path, p_parent->path );
	strcpy( p_child->name, p_parent->name );
	strcpy( p_child->working_dir, p_parent->working_dir );
	
	memcpy( &p_parent->context, &(**context), sizeof(registers) );
	memcpy( &p_child->context, &p_parent->context, sizeof(registers) );

	p_child->context.rip = start;

	// Stack setup

	p_child->proc_stack_virt = 0x00000000A0000000;
	p_child->proc_stack_kvirt = page_allocate_kernel( PROCESS_DEFAULT_STACK_PAGES );
	p_child->proc_stack_phys = paging_virtual_to_physical( p_child->proc_stack_kvirt );
	p_child->stack_size = PROCESS_DEFAULT_STACK_SIZE;
	memcpy( p_child->proc_stack_kvirt, p_parent->proc_stack_kvirt, p_parent->stack_size );
	p_child->context.rsp = (uint64_t)p_child->proc_stack_virt + p_child->stack_size - 8;

	// Symbol index setup
	p_child->rela_sym_index = p_parent->rela_sym_index;
	p_child->dyn_sym_index = p_parent->dyn_sym_index;
	p_child->num_dyn_syms = p_parent->num_dyn_syms;

	p_child->status = PROCESS_STATUS_INACTIVE;

	klog( LOG_DEBUG, "Out clone. Returning %d", pid_child );

	(*context)->rax = p_child;
	//(*context)->rip = start;

	debugf( "Out clone. Returning to %d @ 0x%016llX\n", pid_child, (*context)->rip );

	process_set_next_up( p_child );
	process_sched_yield( context );

	return 0;
}