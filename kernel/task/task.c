#include <kernel_common.h>
#include <kmemory.h>
#include <task.h>
#include <timer.h>
#include <kshell.h>
#include <program.h>

kernel_process_data process_data;
uint16_t task_id_top;

void task_initalize( void ) {
	task_id_top = 1000;

	// Setup the kernel task
	process_data.tasks = kmalloc( sizeof(task) );
	process_data.tasks->id = 0;
	process_data.tasks->status = TASK_STATUS_ACTIVE;
	process_data.tasks->next = NULL;
	process_data.tasks->type = TASK_TYPE_KERNEL;
	process_data.tasks->has_data_ready = false;
	strcpy( process_data.tasks->display_name, "Kernel" );
	strcpy( process_data.tasks->file_name, "kernel.bin" );
	memset( &process_data.tasks->task_context, 0, sizeof(registers) );

	process_data.current_task_id = 0;
	process_data.yield_to_next = 0;

	program_initalize();
}

/**
 * @brief Creates a task, sets status to READY
 * 
 * @param task_type Type of task
 * @param name Dispaly name of task
 * @param entry Entry point
 * @return uint16_t task id
 */
uint16_t task_create( uint8_t task_type, uint8_t generator, char *name, uint64_t *entry ) {
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
	new_task->parent_task_id = process_data.current_task_id;
	new_task->status = TASK_STATUS_READY;
	new_task->type = task_type;
	new_task->next = NULL;
	new_task->entry = (task_entry_func)entry;
	new_task->has_data_ready = false;
	strcpy( new_task->display_name, name );
	strcpy( new_task->file_name, "something.bin" );

	memset( &new_task->task_context, 0, sizeof(registers) );
	new_task->task_context.cs = 0x28;
	new_task->task_context.rflags = 0x200;
	new_task->task_context.rsp = (uint64_t)kmalloc( 4 * 1024 ) + 4*1024;
	new_task->task_context.rip = (uint64_t)entry;

	switch( generator ) {
		case TASK_GENERATOR_ELF:
			break;
		default:
			// Nothing?
	}

	debugf( "Task created: ID: %d, Name: \"%s\"\n", new_task->id, new_task->display_name );

	return task_id;
}

#undef DEBUG_TASK_SCHED_YIELD
void task_sched_yield( registers **context ) {
	#ifdef DEBUG_TASK_SCHED_YIELD
	debugf_raw( "\n================================================================================\n" );
	log_entry_enter();
	#endif

	task *old_task = get_task_data( process_data.current_task_id );
	task *new_task = NULL;

	// If we don't have a next task set, round robin
	if( process_data.yield_to_next == 0 ) {
		bool task_found = false;
		task *task_head = old_task;

		// after this loop, task_head will always contain a good task to switch to
		while( task_found == false ) {
			if( task_head->next == NULL ) {
				task_head = process_data.tasks;
			} else {
				task_head = task_head->next;
			}

			// there's a better way to do this...
			switch( task_head->status ) {
				case TASK_STATUS_READY:
				case TASK_STATUS_ACTIVE:
				case TASK_STATUS_WAIT:
				case TASK_STATUS_INACTIVE:
					task_found = true;
					break;
				case TASK_STATUS_DEAD:
					task_found = false;
					break;
			}
		}
		
		// go to the new task
		new_task = task_head;
	} else {
		new_task = get_task_data( process_data.yield_to_next );
		process_data.yield_to_next = 0;
	}

	memcpy( &old_task->task_context, &(**context), sizeof(registers) );
	#ifdef DEBUG_TASK_SCHED_YIELD
	debugf( "Old task id: %d\n", old_task->id );
	debugf( "Old task saved context:\n" );
	//task_dump_context( &old_task->task_context );
	#endif

	memcpy( &(**context), &new_task->task_context, sizeof(registers) );
	#ifdef DEBUG_TASK_SCHED_YIELD
	debugf( "New task id: %d\n", new_task->id );
	debugf( "New context to use:\n" );
	task_dump_context( *context );
	#endif

	process_data.current_task_id = new_task->id;

	if( old_task->status != TASK_STATUS_DEAD ) {
		old_task->status = TASK_STATUS_INACTIVE;
	}
	new_task->status = TASK_STATUS_ACTIVE;
	

	#ifdef DEBUG_TASK_SCHED_YIELD
	log_entry_exit();
	debugf_raw( "================================================================================\n" );
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
		if( head->id == task_id ) {
			ret_val = head;
			head = NULL;
		} else {
			head = head->next;
		}
	} while( head != NULL );

	return ret_val;
}

void task_set_has_data_ready( uint16_t task_id, bool ready ) {
	task *t = get_task_data( task_id );

	if( t == NULL ) {
		debugf( "Setting task ready for non-existant task: %d\n", task_id );
		return;
	}

	t->has_data_ready = ready;
}

uint16_t task_get_current_task_id( void ) {
	return process_data.current_task_id;
}

void task_set_task_status( uint16_t task_id, uint8_t status ) {
	task *t = get_task_data( task_id );

	if( t == NULL ) {
		debugf( "Setting task status for non-existant task: %d\n", task_id );
		return;
	}

	t->status = status;
}

/**
 * @brief On next yield, the given task id will be switched to
 * 
 * @param task_id ID of task to run next
 */
void task_set_yield_to_next( uint16_t task_id ) {
	process_data.yield_to_next = task_id;
}

/**
 * @brief Exit the task cleanly.
 * 
 * Set the task type to dead (can be cleaned up later).
 * Yield next to the kernel
 * Yield
 * 
 * @param task_id 
 */
void task_exit( uint16_t task_id, uint16_t parent_task_id ) {
	task_set_task_status( task_id, TASK_STATUS_DEAD );
	
	task_set_yield_to_next( parent_task_id );
	
	//debugf( "Yielding to task id: %d\n", parent_task_id );
	
	syscall( SYSCALL_SCHED_YIELD, 0, NULL );
}

void task_launch_kernel_thread( uint64_t *entry, char *name, int argc, char *argv[] ) {

}

void task_exec_syscall_handler( registers **context, uint16_t task_id, int argc, char *argv[] ) {
	task *t = get_task_data( task_id );
	t->task_context.rip = (uint64_t)task_environment_preamble;
	t->argc = argc;
	t->argv = argv;

	task_set_yield_to_next( task_id );

	task_sched_yield( context );
}

void task_environment_preamble( void ) {
	uint16_t task_id = process_data.current_task_id;
	
	task *t = get_task_data( task_id );

	kshell_main_func_to_call main_func = (kshell_main_func_to_call)t->entry;
	uint16_t parent_task_id = t->parent_task_id;

	t->exit_code = main_func( t->argc, t->argv );

	//dfv( task_id );

	task_exit( task_id, t->parent_task_id );
}

uint64_t task_get_exit_code( uint16_t task_id ) {
	task *t = get_task_data( task_id );

	return t->exit_code;
}