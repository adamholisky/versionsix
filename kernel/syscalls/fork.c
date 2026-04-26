#include <kernel_common.h>
#include <stdint.h>
#include <stddef.h>
#include <syscall.h>
#include <process.h>
#include <kmemory.h>
#include <page.h>

pid_t fork_syscall_handler( registers **context ) {
	klog( LOG_DEBUG, "In fork." );

	process_data *p_parent = process_get_current();
	pid_t pid_parent = p_parent->pid;

	printf( "pid_parent: %d\n", pid_parent );

	pid_t pid_child = process_get_new_process();

	printf( "pid_child: %d\n", pid_child );
	

	process_data *p_child = process_get_data_from_pid( pid_child );

	printf( "pid_child: %d\n", pid_child );
	printf( "p_child: %016llX\n", p_child );

	if( p_child == NULL ) {
		klog( LOG_PANIC, "p_child is null." );
		do_immediate_shutdown();
	}

	

	p_child->argc = p_parent->argc;
	p_child->argv = p_parent->argv;
	p_child->binary_format_data = p_parent->binary_format_data;
	p_child->exec_size = p_parent->exec_size;
	p_child->has_own_addr_space = p_parent->has_own_addr_space;
	p_child->pid_parent = p_parent->pid;

	

	strcpy( p_child->path, p_parent->path );
	strcpy( p_child->name, p_parent->name );
	strcpy( p_child->working_dir, p_parent->working_dir );
	
	memcpy( &p_parent->context, &(**context), sizeof(registers) );
	memcpy( &p_child->context, &p_parent->context, sizeof(registers) );

	// Stack setup
	//p_child->proc_stack = kmalloc(p_parent->stack_size);
	//p_child->stack_size = p_parent->stack_size;

	

	p_child->proc_stack = page_allocate_kernel(1);
	p_child->stack_size = 0x1000;
	memcpy( p_child->proc_stack, p_parent->proc_stack, p_parent->stack_size );
	uint64_t stack_offset = (uint64_t)p_parent->proc_stack - (uint64_t)p_parent->context.rsp;
	p_child->context.rsp = p_child->proc_stack + stack_offset;

	// RBP setup.
	// TODO: I'm not sure of this, research and verify
	//uint64_t parent_rbp_offset = p_parent->context.rbp - p_parent->context.rsp;
	//p_child->context.rbp = p_child->context.rsp + parent_rbp_offset;

	//processes_diagnostic_dump();

	// Data section setup
	p_child->data_section_count = p_parent->data_section_count;
	//p_child->data_sections = kmalloc( sizeof(process_exec_section) * p_child->data_section_count );
	p_child->data_sections = page_allocate_kernel(1);
	p_child->data_section_virt_start = p_parent->data_section_virt_start;

	for( int i = 0; i < p_child->data_section_count; i++ ) {
		p_child->data_sections[i].kern_virt = page_allocate_kernel(1);
		p_child->data_sections[i].virt = p_parent->data_sections[i].virt;
		p_child->data_sections[i].phys = paging_virtual_to_physical( p_child->data_sections[i].kern_virt );

		void *child_ds_page_kern_virt = p_child->data_sections[i].kern_virt;

		debugf( "c_ds_kv: 0x%016llX\n", child_ds_page_kern_virt );

		memcpy( child_ds_page_kern_virt, p_parent->data_sections[i].kern_virt, PAGE_SIZE );
	}

	// Text section setup
	p_child->text_section_count = p_parent->text_section_count;
	p_child->text_sections = page_allocate_kernel(1);
	//p_child->text_sections = kmalloc( sizeof(process_exec_section) * p_child->text_section_count );
	p_child->text_secton_virt_start = p_parent->text_secton_virt_start;

	for( int i = 0; i < p_child->text_section_count; i++ ) {
		p_child->text_sections[i].kern_virt = page_allocate_kernel(1);
		p_child->text_sections[i].virt = p_parent->text_sections[i].virt;
		p_child->text_sections[i].phys = paging_virtual_to_physical( p_child->text_sections[i].kern_virt );

		void *child_ts_page_kern_virt = p_child->text_sections[i].kern_virt;

		debugf( "c_ts_kv: 0x%016llX\n", child_ts_page_kern_virt );

		memcpy( child_ts_page_kern_virt, p_parent->text_sections[i].kern_virt, PAGE_SIZE );
	}

	// Symbol index setup
	p_child->rela_sym_index = p_parent->rela_sym_index;
	p_child->dyn_sym_index = p_parent->dyn_sym_index;
	p_child->num_dyn_syms = p_parent->num_dyn_syms;

	p_child->status = PROCESS_STATUS_INACTIVE;

	//processes_diagnostic_dump();

	p_child->context.rax = 0;

	klog( LOG_DEBUG, "Out fork. Returning %d", pid_child );

	(*context)->rax = pid_child;


	printf( "Parent context:\n" );
	process_diagnostic_context( *context );

	printf( "Child context:\n" );
	process_diagnostic_context( &p_child->context );	

	return pid_child;
}