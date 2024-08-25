#ifndef VIOS_TESTS_INCLUDED
#define VIOS_TESTS_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint16_t width;
	uint16_t height;
	uint8_t *image;
} vui_sprite;

void tests_run_tests( void );
void tests_run_program( void );	
void tests_fps( void );
void save_rect( uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *store );
void load_rect( uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *store );
void set_frame_start( void );
void wait_for_next_frame( uint16_t target_fps );
void animate_sprite_x_y( vui_sprite *sprite, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t seconds );
void tests_animation( void );
int dumb_rand( int r );
void random_seed( uint64_t seed );

#ifdef __cplusplus
}
#endif
#endif