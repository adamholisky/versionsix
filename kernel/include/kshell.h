#ifndef VIOS_KSHELL_INCLUDED
#define VIOS_KSHELL_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define KSHELL_MAX_HISTORY 25
#define KSHELL_MAX_LINESIZE 255

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

void kshell_initalize( void );

#ifdef __cplusplus
}
#endif
#endif