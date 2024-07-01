#include <kernel_common.h>
#include <kmemory.h>
#include <task.h>

Task * tasks[ TASKS_MAX ];

Task::Task( uint16_t task_id, uint8_t task_type, char *task_name, uint64_t *task_entry ) {
	log_entry_enter();

	id = task_id;
	type = task_type;
	strcpy( display_name, task_name );
	entry = (task_entry_func)task_entry;
	status = TASK_STATUS_READY;

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

void task_initalize( void ) {
	for( int i = 0; i < TASKS_MAX; i++ ) {
		tasks[i] = NULL;
	}

	Task *kernel_task = task_create( TASK_TYPE_KERNEL, "Kernel", NULL );
	Task *kernel_thread_a = task_create( TASK_TYPE_KERNEL_THREAD, "Thread A", (uint64_t *)task_test_thread_a );
	Task *kernel_thread_b = task_create( TASK_TYPE_KERNEL_THREAD, "Thread B", (uint64_t *)task_test_thread_b ); 

	kernel_thread_a->start();
	kernel_thread_b->start();
}

void task_test_thread_a( void ) {
	debugf( "Test Thread A\n" );
}

void task_test_thread_b( void ) {
	debugf( "Test Thread B\n" );
}