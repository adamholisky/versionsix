#ifndef VIOS_KSHELL_INCLUDED
#define VIOS_KSHELL_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define KSHELL_MAX_HISTORY 25
#define KSHELL_MAX_LINESIZE 255
#define KSHELL_MAX_ARGS 10

#define KSHELL_EXIT_CODE_SUCCESS 0
#define KSHELL_EXIT_CODE_FAILURE 1

typedef int (*kshell_main_func_to_call)( int num_args, char *arg_list[] );

typedef struct {
	bool keep_going;
	char *lines[KSHELL_MAX_HISTORY];
	char *current_line;
	uint8_t line_index;
} kshell_config;

typedef struct {
	char name[64];
	void *entry;
} kshell_command;

typedef struct {
	void *next;
	kshell_command *cmd;
} kshell_command_list;

typedef struct {
	char name[50];
	char value[255];
	void *next;
} kshell_env_var;

typedef struct {
	uint16_t num_vars;
	kshell_env_var *top;
} kshell_env_var_list;

void kshell_initalize( void );
void kshell_add_command( char *command_name, void *main_function );
kshell_command *kshell_command_create( char *command_name, void *main_function );
int kshell_command_run( kshell_command *cmd, int argc, char *argv[] );
void kshell_run( void );
void kshell_stop( void );
void kshell_main_loop( void );
bool kshell_handle_special_keypress( uint8_t scancode );

char *kshell_get_env_var( char *name );
void kshell_set_env_var( char *name, char *value );

int kshell_command_hello_world( int argc, char *argv[] );

#ifdef __cplusplus
}
#endif
#endif