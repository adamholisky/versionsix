#ifndef VIOS_TASK_INCLUDED
#define VIOS_TASK_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <interrupt.h>

#define TASKS_MAX 20
#define TASKS_NAME_MAX 25

/** 
 * Task Types:
 * 
 * TASK_TYPE_PROCESS -- Regular user process, owns its address space, code starts at 0
 * TASK_TYPE_THREAD -- Regular thread of a user process, shares address space with parent
 * TASK_TYPE_KERNEL -- The kernel, only one and starts at boot
 * TASK_TYPE_KERNEL_THREAD -- Thread belonging to the kernel, shares kernel address space
 * 
 */
#define TASK_TYPE_PROCESS 1
#define TASK_TYPE_THREAD 2
#define TASK_TYPE_KERNEL 3
#define TASK_TYPE_KERNEL_THREAD 4

#define TASK_STATUS_READY 1
#define TASK_STATUS_ACTIVE 2
#define TASK_STATUS_WAIT 3
#define TASK_STATUS_INACTIVE 4
#define TASK_STATUS_DEAD 5

typedef void (*task_entry_func)( void );

class Task {
	protected:
		uint8_t type;
		task_entry_func entry;
	public:
		char display_name[TASKS_NAME_MAX];
		char file_name[TASKS_NAME_MAX];
		
		uint16_t id;
		uint8_t status;
		registers task_context;

		Task( uint16_t task_id, uint8_t task_type, char *task_name, uint64_t *task_entry );

		void read( void );
		void write( void );

		void start( void );
		void save_context( registers **_context );
		registers *Task::get_context( void );
};

typedef struct {
	Task *tasks[ TASKS_MAX ];
	uint16_t current_task;
	registers task_contexts[ TASKS_MAX ]; 
} kernel_process_data;

void task_initalize( void );
void task_sched_yield( registers **context );
void task_test_thread_a( void );
void task_test_thread_b( void );
void task_dump_context( registers *context );
kernel_process_data *task_get_kernel_process_data( void );

#ifdef __cplusplus
}
#endif
#endif