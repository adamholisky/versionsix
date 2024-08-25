#include <kernel_common.h>
#include <tests.h>
#include <program.h>
#include <page.h>
#include <page_group.h>
#include <task.h>
#include <kmemory.h>

void test_page_group( void );

void tests_run_tests( void ) {
	//test_page_group();
	//tests_fps();
	//tests_run_program();

	tests_animation();

	/* debugf( "End of run tests. Shutting down.\n" );
	do_immediate_shutdown(); */
}

void tests_run_program( void ) {
	program *p = program_load( "/modules/first.o" );
	task_create_from_program( p );
}

void test_page_group( void ) {
	extern page_group main_page_group;
	extern uint64_t kernel_heap_physical_memory_next;

	printf( "pg_page_bitmap:       0x%016llX\n", &main_page_group.page_bitmap );
	printf( "pg_physical_base:     0x%016llX\n", main_page_group.physical_base );
	printf( "pg_num_pages:         0x%016llX\n", main_page_group.num_pages );
	printf( "pg_page_size:         0x%016llX\n\n", main_page_group.page_size );

	printf( "phys_mem_next:        0x%016llX\n", kernel_heap_physical_memory_next );

	uint64_t allocated_mem = page_group_allocate_next_free( &main_page_group );

	printf( "allocated_mem:        0x%016llX\n", allocated_mem );
}

extern uint64_t system_count;
extern vui_core vui;

void tests_fps( void ) {
	uint64_t fps_seconds_to_sample = 10;
	uint64_t total_frames = 0;

	debugf( "system_count: %lld\n", system_count );

	uint32_t y = 50;

	for( int i = 0; i < fps_seconds_to_sample; i++ ) {
		uint64_t timer_start = system_count;
		uint64_t frames_rendered = 0;

		uint32_t x = 50;
		uint32_t screen_copy[100*100];

		save_rect( x, y, 100, 100, screen_copy );
		bool first = true;

		do {
			set_frame_start();

			if( !first ) {
				load_rect( x - 10, y, 100, 100, screen_copy );
			}

			first = false;
			
			save_rect( x, y, 100, 100, screen_copy );

			vui_draw_rect( x, y, 100, 100, COLOR_RGB_RED );
			vui_refresh();

			x = x + 10;
			//debugf( "system_count: %lld\n", system_count );

			frames_rendered++;

			wait_for_next_frame( 30 );
		} while( system_count <= timer_start + 100 );

		load_rect( x - 10, y, 100, 100, screen_copy );
		vui_refresh();

		total_frames = total_frames + frames_rendered;

		y += 50;

		printf( "FPS Sample %d: %d frames\n", i, frames_rendered );
	}

	uint64_t fps = total_frames / fps_seconds_to_sample;
	
	printf( "FPS average: %d\n", fps );
	debugf( "FPS average: %d\n", fps );
}

void tests_animation( void ) {
	vui_sprite *sprite = kmalloc( sizeof(vui_sprite) );

	sprite->height = 50;
	sprite->width = 50;
	sprite->image = kmalloc( sizeof(50*50*4) );

	for( uint32_t i = 0; i < 50*50; i++ ) {
		*((uint32_t *)(sprite->image) + i) = COLOR_RGB_BLUE;
	}

	animate_sprite_x_y( sprite, 0, 0, 1920-50, 1080-50, 10 );
}

void animate_sprite_x_y( vui_sprite *sprite, uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t seconds ) {
	uint16_t frames = seconds * 34;
	uint16_t x_per_frame = (x_end - x_start)/frames;
	uint16_t y_per_frame = (y_end - y_start)/frames;

	printf( "xpf: 0x%X\n", x_per_frame );
	printf( "ypf: 0x%X\n", y_per_frame );

	uint32_t x = x_start;
	uint32_t y = y_start;

	uint32_t screen_copy[ sprite->width * sprite->height ];

	bool first = true;

	for( uint64_t i = 0; i < frames; i++ ) {
		set_frame_start();

		if( !first ) {
			load_rect( x - x_per_frame, y - y_per_frame, sprite->width, sprite->height, screen_copy );
			vui_refresh_rect( x - x_per_frame, y - y_per_frame, sprite->width, sprite->height );			
		} else {
			first = false;
		}

		save_rect( x, y, sprite->width, sprite->height, screen_copy );

		load_rect( x, y, sprite->width, sprite->height, sprite->image );
		vui_refresh_rect( x, y, sprite->width, sprite->height );

		x = x + x_per_frame;
		y = y + y_per_frame;

		wait_for_next_frame( 30 );
	} 

	load_rect( x - x_per_frame, y - y_per_frame, sprite->width, sprite->height, screen_copy );
	load_rect( x_end, y_end, sprite->width, sprite->height, sprite->image );
	vui_refresh();
}

uint64_t frame_start_system_count;

void set_frame_start( void ) {
	frame_start_system_count = system_count;
}

void wait_for_next_frame( uint16_t target_fps ) {
	uint64_t frame_period = 100/target_fps;

	uint64_t x = 0;

	while( system_count < frame_period + frame_start_system_count ) {
		x++;
	}

	if( x == 0 ) {
		debugf( "*************************************Frame dropped?\n" );
	}
}

void save_rect( uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *store ) {
	for( int z = 0; z < height; z++ ) {
		memcpy( store + (z * width * 4), vui.buffer + ((y+z) * (vui.pitch/4)) + x, width*4 );
	}
}

void load_rect( uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *store ) {
	for( int z = 0; z < height; z++ ) {
		memcpy( vui.buffer + ((y + z) * (vui.pitch/4)) + x, store + (z * width * 4), width*4 );
	}
}