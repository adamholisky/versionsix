#include <kernel_common.h>
#include <kmemory.h>
#include <task.h>
#include <timer.h>

kernel_process_data process_data;
uint16_t task_id_top;

void task_initalize( void ) {
	task_id_top = 1000;

	// Setup the kernel task
	process_data.tasks = kmalloc( sizeof(task) );
	process_data.tasks->id = 0;
	strcpy( process_data.tasks->display_name, "Kernel" );
	strcpy( process_data.tasks->file_name, "kernel.bin" );
	process_data.tasks->status = TASK_STATUS_ACTIVE;
	memset( &process_data.tasks->task_context, 0, sizeof(registers) );
	process_data.tasks->next = NULL;
	process_data.tasks->type = TASK_TYPE_KERNEL;

	process_data.current_task_id = 0;
}

/**
 * @brief Creates a task, sets status to READY
 * 
 * @param task_type Type of task
 * @param name Dispaly name of task
 * @param entry Entry point
 * @return uint16_t task id
 */
uint16_t task_create( uint8_t task_type, char *name, uint64_t *entry ) {
	uint16_t task_id = 0;

	if( task_type == TASK_TYPE_KERNEL ) {
		return 0;
	}

	task_id = task_id_top++;
	
	task *new_task = process_data.tasks;
	task *old_last_task = NULL;
	do {
		old_last_task = new_task;
		new_task = new_task->next;
	} while( new_task != NULL );

	new_task = kmalloc( sizeof(task) );
	old_last_task->next = new_task;

	new_task->id = task_id;
	new_task->status = TASK_STATUS_READY;
	new_task->type = task_type;
	new_task->next = NULL;
	new_task->entry = (task_entry_func)entry;
	strcpy( new_task->display_name, name );
	strcpy( new_task->file_name, "something.bin" );

	memset( &new_task->task_context, 0, sizeof(registers) );
	new_task->task_context.rip = (uint64_t)entry;
	new_task->task_context.cs = 0x28;
	new_task->task_context.rflags = 0x200;
	new_task->task_context.rsp = (uint64_t)kmalloc( 4 * 1024 ) + 4*1024;

	debugf( "Task created: \"%s\"\n", new_task->display_name );

	return task_id;
}

#undef DEBUG_TASK_SCHED_YIELD
void task_sched_yield( registers **context ) {
	#ifdef DEBUG_TASK_SCHED_YIELD
	log_entry_enter();
	#endif

	task *old_task = get_task_data( process_data.current_task_id );
	task *new_task = NULL;

	dfv( old_task->id );

	if( old_task->next == NULL ) {
		new_task = process_data.tasks;
	} else {
		new_task = old_task->next;
	}

	memcpy( &old_task->task_context, &(**context), sizeof(registers) );
	#ifdef DEBUG_TASK_SCHED_YIELD
	debugf( "Old task id: %d\n", old_task->id );
	debugf( "Old task saved context:\n" );
	task_dump_context( &old_task->task_context );
	#endif

	memcpy( &(**context), &new_task->task_context, sizeof(registers) );
	#ifdef DEBUG_TASK_SCHED_YIELD
	debugf( "New task id: %d\n", new_task->id );
	debugf( "New context to use:\n" );
	task_dump_context( *context );
	#endif

	process_data.current_task_id = new_task->id;

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

task *get_task_data( uint16_t task_id ) {
	task *head = process_data.tasks;
	task *ret_val = NULL;

	do {
		dfv( head->id );
		if( head->id == task_id ) {
			ret_val = head;
			head = NULL;
		} else {
			head = head->next;
		}
	} while( head != NULL );

	return ret_val;
}

void task_set_task_status( uint16_t task_id, uint8_t status ) {

}

void task_launch_kernel_thread( uint64_t *entry, char *name, int argc, char *argv[] ) {

}