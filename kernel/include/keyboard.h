#ifndef VIOS_KEYBOARD_INCLUDED
#define VIOS_KEYBOARD_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <interrupt.h>
#include <stdbool.h>

#define SCANCODE_ESC 0x01
#define SCANCODE_F1 0x3B
#define SCANCODE_F2 0x3C

#define SCANCODE_ESC_RELEASE 0x81

#define SCANCODE_PAGE_UP 0x49
#define SCANCODE_PAGE_DOWN 0x51

class Keyboard {
	private:
		bool is_waiting;
		bool is_shift;
		bool in_E0;
		uint8_t current_scancode;
		uint8_t scancode_queue[255];
		uint8_t scancode_queue_head;
		uint8_t scancode_queue_tail;
	public:
		bool char_ready;

		Keyboard( void );
		void interrupt_handler( void );
		
		void add_scancode_to_queue( uint8_t code );
		uint8_t get_next_scancode( void );
		char get_next_char( bool return_special = false );
};

void keyboard_initalize( void );
void keyboard_interrupt_handler( registers **reg );
char keyboard_get_char( void );
char keyboard_get_char_or_special( void );
char keyboard_get_char_stage_2( bool return_special );
char scancode_to_char( uint8_t scancode );

#ifdef __cplusplus
}
#endif
#endif