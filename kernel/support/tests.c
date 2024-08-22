#include <kernel_common.h>
#include <tests.h>
#include <program.h>
#include <page.h>
#include <page_group.h>
#include <task.h>

void test_page_group( void );

void tests_run_tests( void ) {
	test_page_group();
	//tests_run_program();

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