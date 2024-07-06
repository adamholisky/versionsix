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

class KShell {
	private:
		bool keep_going;
		char *lines[KSHELL_MAX_HISTORY];
		char *current_line;
		uint8_t line_index;
	public:
		KShell( void );
		void run( void );
		void stop( void );
		void main_loop( void );
		bool handle_special_keypress( uint8_t scancode );
};

/* class KShell_Command {
	public:
		char name[64];
		void *entry;

		KShell_Command( char *command_name, void *main_function );
		~KShell_Command();
		KShell_Command& operator=(const KShell_Command orig);
		int run( int argc, char *argv[] );
};*/

typedef struct {
	char name[64];
	void *entry;
} kshell_command;

typedef struct {
	void *next;
	kshell_command *cmd;
} kshell_command_list;

void kshell_initalize( void );
void kshell_add_command( char *command_name, void *main_function );
kshell_command *kshell_command_create( char *command_name, void *main_function );
int kshell_command_run( kshell_command *cmd, int argc, char *argv[] );

#define KSHELL_COMMAND( name, main_function ) \
	extern "C" int kshell_app_ ##name## _main( int c, char *argv[] ); \
	extern "C" void kshell_app_add_command_ ##name ( void ) { \
		kshell_add_command( #name, (void *)main_function ); \
	}

#ifdef __cplusplus
}
#endif
#endif