#include <kernel_common.h>
#include <kmemory.h>
#include <task.h>
#include <timer.h>

kernel_process_data process_data;

void task_initalize( void ) {
	memset( &process_data.tasks, 0, sizeof( task ) * TASKS_MAX );

	task_create( TASK_TYPE_KERNEL, "Kernel", NULL );
	/* Task *kernel_thread_a = task_create( TASK_TYPE_KERNEL_THREAD, "Thread A", (uint64_t *)task_test_thread_a );
	Task *kernel_thread_b = task_create( TASK_TYPE_KERNEL_THREAD, "Thread B", (uint64_t *)task_test_thread_b ); */

	process_data.current_task = 0;
}

uint16_t task_create( uint8_t task_type, char *name, uint64_t *entry ) {
	uint16_t task_id = 0;

	if( task_type != TASK_TYPE_KERNEL ) {
		for( int i = 0; i < TASKS_MAX; i++ ) {
			if( process_data.tasks[i].status == TASK_STATUS_NOT_CREATED ) {
				task_id = i;
				i = TASKS_MAX;
			}
		}

		if( task_id == 0 ) {
			debugf( "Cannot find free task id.\n" );
			return 0;
		}
	}

	process_data.tasks[task_id].id = task_id;
	process_data.tasks[task_id].type = task_type;
	memset( process_data.tasks[task_id].display_name, 0, TASKS_NAME_MAX );
	strcpy( process_data.tasks[task_id].display_name, name );

	debugf( "\"%s\"", process_data.tasks[task_id].display_name );
	process_data.tasks[task_id].entry = (task_entry_func)entry;
	process_data.tasks[task_id].status = TASK_STATUS_READY;

	memset( &process_data.task_contexts[task_id], 0, sizeof( registers ) );

	process_data.task_contexts[task_id].rip = (uint64_t)entry;
	process_data.task_contexts[task_id].cs = 0x28;
	process_data.task_contexts[task_id].rflags = 0x200;
	process_data.task_contexts[task_id].rsp = (uint64_t)kmalloc( 4 * 1024 ) + 4*1024;
	//process_data.task_contexts[task_id].rax = 0xAAAAAAAAAAAAAAAA;

	if( task_type == TASK_TYPE_KERNEL ) {
		process_data.tasks[task_id].status = TASK_STATUS_ACTIVE;
	}

	return task_id;
}

#undef DEBUG_TASK_SCHED_YIELD
void task_sched_yield( registers **context ) {
	#ifdef DEBUG_TASK_SCHED_YIELD
	log_entry_enter();
	#endif

	uint32_t old_task_number = process_data.current_task;
	uint32_t new_task_number = process_data.current_task + 1;

	if( new_task_number == 3 ) {
		new_task_number = 0;
	}
	
	#ifdef DEBUG_TASK_SCHED_YIELD
	debugf( "Old task number: \t\t%d\n", old_task_number );
	debugf( "New task number: \t\t%d\n", new_task_number );

	debugf( "Old task saved context:\n" );
	registers *old_task_context = &task_contexts[old_task_number];
	task_dump_context( old_task_context );
	#endif

	memcpy( &process_data.task_contexts[old_task_number], &(**context), sizeof(registers) );
	//process_data.task_contexts[old_task_number].rax = 0xBBBBBBBBBBBBBBBB;
	//process_data.task_contexts[old_task_number].rbp = (**context).rbp;

	#ifdef DEBUG_TASK_SCHED_YIELD
	debugf( "Old task saved :\n" );
	task_dump_context( old_task_context );

	debugf( "saved context for task %d:\n", new_task_number );
	task_dump_context( &task_contexts[new_task_number] );
	#endif

	memcpy( &(**context), &process_data.task_contexts[new_task_number], sizeof(registers) );

	#ifdef DEBUG_TASK_SCHED_YIELD
	debugf( "New context to use:\n" );
	task_dump_context( *context );
	#endif

	process_data.current_task = new_task_number;

	#ifdef DEBUG_TASK_SCHED_YIELD
	log_entry_exit();
	#endif
}

kernel_process_data *task_get_kernel_process_data( void ) {
	return &process_data;
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

void task_test_thread_a( void ) {
	debugf( "Test Thread A\n" );

	while( true ) {
		debugf_raw( "A" );
		timer_wait(1);
		syscall( SYSCALL_SCHED_YIELD, 0, NULL );
	}
}

void task_test_thread_b( void ) {
	debugf( "Test Thread B\n" );

	while( true ) {
		debugf_raw( "B" );
		timer_wait(1);
		syscall( SYSCALL_SCHED_YIELD, 0, NULL );
	}
}