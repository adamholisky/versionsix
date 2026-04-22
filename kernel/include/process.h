#if !defined(PROCESS_H_INCLUDED)
#define PROCESS_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define ROOT_PROCESS_ID 0

typedef uint32_t pid_t;

typedef struct {
	uint64_t phys;
	uint64_t virt;
	uint64_t kern_virt;
} process_exec_section;

typedef struct {
	pid_t pid;
	void *entry;
	char path[1024];
	uint32_t exec_size;

	int argc;

	uint16_t text_section_count;
	process_exec_section *text_sections;
	uint64_t text_secton_virt_start;

	uint16_t data_section_count;
	process_exec_section *data_sections;
	uint64_t data_section_virt_start;

	void *binary_format_data;
} process_data;

void process_initalize( void );
pid_t process_create( void );
void process_start_root_process( void );

void process_env_setup( void );
void process_exit( int ret_code );

int process_idle_loop_entry( int argc, char *argv[] );
void process_idle_loop( void );

pid_t vios_fork( void );

#ifdef __cplusplus
}
#endif

#endif