#ifndef VIOS_KEYBOARD_INCLUDED
#define VIOS_KEYBOARD_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <interrupt.h>
#include <stdbool.h>

class Keyboard {
	private:
		bool is_waiting;
		bool is_shift;
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
		char get_next_char( void );
};

void keyboard_initalize( void );
void keyboard_interrupt_handler( registers **reg );
char keyboard_get_char( void );

#ifdef __cplusplus
}
#endif
#endif