#if !defined(PROCESS_H_INCLUDED)
#define PROCESS_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <ksymbols.h>
#include <lib/list.h>

#define KERNEL_PROCESS_ID 0
#define ROOT_PROCESS_ID 1

#define PROCESS_DEFAULT_STACK_SIZE 65536
#define PROCESS_DEFAULT_STACK_PAGES 16

#define PROCESS_STATUS_IN_SETUP 0
#define PROCESS_STATUS_ACTIVE 1
#define PROCESS_STATUS_INACTIVE 2
#define PROCESS_STATUS_WAITING 3
#define PROCESS_STATUS_SLEEP 4
#define PROCESS_STATUS_WAITING_FOR_DEATH 5
#define PROCESS_STATUS_DEAD 6

typedef uint32_t pid_t;

typedef struct {
	uint64_t phys;
	uint64_t virt;
	uint64_t *kern_virt;
} process_exec_section;

typedef struct {
	uint16_t elf_index;
	char name[255];
	uint64_t value;
	uint64_t offset;
	uint64_t info;
	uint64_t size;
} symbol_index;

typedef struct {
	pid_t pid;
	pid_t pid_parent;

	uint8_t status;
	int exit_code;
	char name[256];
	registers context;
	bool first_run;
	pid_t wait_for_pid;

	void *proc_stack_kvirt;
	void *proc_stack_virt;
	void *proc_stack_phys;
	uint32_t stack_size;
	
	int argc;
	char **argv;
	char working_dir[256];

	void *entry;
	char path[256];
	uint32_t exec_size;
	bool has_own_addr_space;

	uint16_t text_section_count;
	process_exec_section *text_sections;
	uint64_t text_secton_virt_start;

	uint16_t data_section_count;
	process_exec_section *data_sections;
	uint64_t data_section_virt_start;
	uint64_t data_section_ava;

	symbol_index *rela_sym_index;
	symbol_index *dyn_sym_index;
	uint16_t num_dyn_syms;

	void *binary_format_data;
} process_data;

typedef struct {
	avs_list *process_list;
	pid_t pid_next;
	process_data *kernel_pd;
	process_data *root_pd;

	process_data *current_process;
	process_data *process_next_up;
} kernel_proc_data;

void process_initalize( void );
pid_t process_create( void );
pid_t process_get_new_process( void );
process_data *process_get_current( void );
pid_t process_get_current_proc_id( void );
void process_setup_init( void );
process_data* process_get_data_from_pid( pid_t pid );
int avs_list_compare_pids( void *a, void *b );

void process_env_setup( void );
void process_exit( int ret_code );

int process_idle_loop_entry( int argc, char *argv[] );
void process_idle_loop( void );
int process_sched_yield( registers **context );
void process_sched_set_next_yield( pid_t pid );
void process_set_next_up( process_data *p );

void processes_diagnostic_dump( void );
void process_diagnostic_context( registers *context );


#ifdef __cplusplus
}
#endif

#endif