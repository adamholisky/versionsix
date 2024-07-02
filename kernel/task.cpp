#include <kernel_common.h>
#include <kmemory.h>
#include <task.h>

Task * tasks[ TASKS_MAX ];
uint16_t current_task;

Task::Task( uint16_t task_id, uint8_t task_type, char *task_name, uint64_t *task_entry ) {
	log_entry_enter();

	id = task_id;
	type = task_type;
	strcpy( display_name, task_name );
	entry = (task_entry_func)task_entry;
	status = TASK_STATUS_READY;

	memset( &task_context, 0, sizeof( registers ) );

	task_context.rip = (uint64_t)entry;
	task_context.cs = 0x28;
	task_context.rflags = 0x200;
	task_context.rsp = (uint64_t)kmalloc( 4 * 1024 );
	task_context.rax = 0xAAAAAAAAAAAAAAAA;

	if( type == TASK_TYPE_KERNEL ) {
		status = TASK_STATUS_ACTIVE;
	}

	log_entry_exit();
}

void Task::start( void ) {
	if( status != TASK_STATUS_READY ) {
		debugf( "Task status is not TASK_STATUS_READY. Aborting start.\n" );
		return;
	}
	
	status = TASK_STATUS_ACTIVE;
	entry();
}

void Task::save_context( registers **_context ) {
	registers *context = *_context;
	debugf( "save reg rbp: %X\n", context->rbp );
	memcpy( &task_context, context, sizeof( registers ) );
}

registers *Task::get_context( void ) {
	return &task_context;
}

void Task::read( void ) {

}

void Task::write( void ) {

}

void task_dump_context( registers *context ) {
	registers *reg = context;

	debugf_raw( "================================================================================\n" );
	debugf_raw( "    rip:  0x%016llX\n", reg->rip );
	debugf_raw( "    rax:  0x%016llX  rbx:  0x%016llX  rcx:  0x%016llX\n", reg->rax, reg->rbx, reg->rcx );
	debugf_raw( "    rdx:  0x%016llX  rsi:  0x%016llX  rdi:  0x%016llX\n", reg->rdx, reg->rsi, reg->rdi );
	debugf_raw( "    rsp:  0x%016llX  rbp:  0x%016llX  cr0:  0x%016llX \n", reg->rsp, reg->rbp, 0 );
	debugf_raw( "    cr2:  0x%016llX  cr3:  0x%016llX  cr4:  0x%016llX\n", 0, 0, 0 );
	debugf_raw( "    cs:   0x%04X  num:  0x%08X  err:  0x%08X  flag: 0x%08X\n", reg->cs, 0, 0, reg->rflags);
	debugf_raw( "================================================================================\n" );
	debugf_raw( "\n" );
}

Task *task_create( uint8_t task_type, char *name, uint64_t *entry ) {
	uint16_t free_id = 0;

	if( task_type != TASK_TYPE_KERNEL ) {
		for( int i = 0; i < TASKS_MAX; i++ ) {
			if( tasks[i] == NULL ) {
				free_id = i;
				i = TASKS_MAX;
			}
		}

		if( free_id == 0 ) {
			debugf( "Cannot find free task id.\n" );
			return NULL;
		}
	}

	tasks[free_id] = new Task( free_id, task_type, name, entry );

	return tasks[free_id];
}

void task_sched_yield( registers **context ) {
	log_entry_enter();

	uint32_t old_task_number = current_task;
	uint32_t new_task_number = current_task + 1;

	if( new_task_number == 3 ) {
		new_task_number = 0;
	}
	
	debugf( "Old task number: \t\t%d\n", old_task_number );
	debugf( "New task number: \t\t%d\n", new_task_number );

	debugf( "Old task saved context:\n" );
	registers *old_task_context = &tasks[old_task_number]->task_context;
	task_dump_context( old_task_context );

	memcpy( &tasks[old_task_number]->task_context, &(**context), sizeof(registers) );
	tasks[old_task_number]->task_context.rax = 0xBBBBBBBBBBBBBBBB;
	tasks[old_task_number]->task_context.rbp = (**context).rbp;

	debugf( "Old task saved :\n" );
	//kdebug_peek_at( (uint64_t)&tasks[current_task]->task_context );
	task_dump_context( old_task_context );

	


	//kdebug_peek_at( (uint64_t)tasks[current_task]->get_context() );
	debugf( "saved context for task %d:\n", new_task_number );
	task_dump_context( &tasks[new_task_number]->task_context );

	memcpy( *context, &tasks[new_task_number]->task_context, sizeof(registers) );

	(*context)->rax = tasks[new_task_number]->task_context.rax;
	(*context)->rip = tasks[new_task_number]->task_context.rip;
	(*context)->rbp = tasks[new_task_number]->task_context.rbp;

	debugf( "New context to use:\n" );
	task_dump_context( *context );

	current_task = new_task_number;

	log_entry_exit();
}

void task_initalize( void ) {
	for( int i = 0; i < TASKS_MAX; i++ ) {
		tasks[i] = NULL;
	}

	Task *kernel_task = task_create( TASK_TYPE_KERNEL, "Kernel", NULL );
	Task *kernel_thread_a = task_create( TASK_TYPE_KERNEL_THREAD, "Thread A", (uint64_t *)task_test_thread_a );
	Task *kernel_thread_b = task_create( TASK_TYPE_KERNEL_THREAD, "Thread B", (uint64_t *)task_test_thread_b ); 

	current_task = 0;
}

void task_test_thread_a( void ) {
	debugf( "Test Thread A\n" );

	while( true ) {
		debugf( "A\n" );
		syscall( SYSCALL_SCHED_YIELD, 0, NULL );
	}
}

void task_test_thread_b( void ) {
	debugf( "Test Thread B\n" );

	while( true ) {
		debugf( "B\n" );
		syscall( SYSCALL_SCHED_YIELD, 0, NULL );
	}
}