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

/*

Task lifetime:

1. Kernel: Created task
    1a. Sets task status to READY
2. Kernel: activates task
	2a. Sets task status to ACTIVE
3. Task: Does stuff
4. Task: Takes a break with:
	4a. Yield -- sets status to INACTIVE
	4b. Wait -- sets status to WAIT
5. Kernel: sets other tasks active
6. Kernel: gets something that should wake up task
    6a. Sets task status to READY
7. Kernel: actives task
    7a. Sets task status to ACTIVE
8. Task: Does stuff (go to step 3)

*/

#define TASK_STATUS_NOT_CREATED 0
#define TASK_STATUS_READY 1
#define TASK_STATUS_ACTIVE 2
#define TASK_STATUS_WAIT 3
#define TASK_STATUS_INACTIVE 4
#define TASK_STATUS_DEAD 5

typedef void (*task_entry_func)( void );

typedef struct _task task;

struct _task {
	uint8_t type;
	task_entry_func entry;

	char display_name[ TASKS_NAME_MAX ];
	char file_name[ TASKS_NAME_MAX ];

	uint16_t id;
	uint8_t status;
	registers task_context;

	bool has_data_ready;

	task *next;
};

typedef struct {
	task *tasks;
	uint16_t current_task_id;
	registers *task_contexts; 
	uint16_t yield_to_next;
} kernel_process_data;

void task_initalize( void );
uint16_t task_create( uint8_t task_type, char *name, uint64_t *entry );
void task_sched_yield( registers **context );
void task_test_thread_a( void );
void task_test_thread_b( void );
void task_dump_context( registers *context );
kernel_process_data *task_get_kernel_process_data( void );
task *get_task_data( uint16_t task_id );
uint16_t task_get_current_task_id( void );
void task_set_has_data_ready( uint16_t task_id, bool ready );
void task_set_task_status( uint16_t task_id, uint8_t status );
void task_set_yield_to_next( uint16_t task_id );
void task_exec( uint64_t *entry, int argc, char *argv[] );

void task_launch_kernel_thread( uint64_t *entry, char *name, int argc, char *argv[] );

#ifdef __cplusplus
}
#endif
#endif