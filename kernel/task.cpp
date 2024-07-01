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

	memset( &task_context, 0, sizeof( task_context ) );

	task_context.rip = (uint64_t)entry;
	task_context.cs = 0x28;
	task_context.rsp = (uint64_t)kmalloc( 4 * 1024 );

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

void Task::save_context( registers *context ) {
	memcpy( &task_context, context, sizeof( registers ) );
}

registers *Task::get_context( void ) {
	return &task_context;
}

void Task::read( void ) {

}

void Task::write( void ) {

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

void task_sched_yield( registers *context ) {
	log_entry_enter();

	tasks[current_task]->save_context( context );

	current_task++;

	if( current_task == 3 ) {
		current_task = 0;
	}

	memcpy( context, tasks[current_task]->get_context(), sizeof(registers) );

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