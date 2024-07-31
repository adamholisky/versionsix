#include <kernel_common.h>
#include <kshell_app.h>
#include <page.h>

KSHELL_COMMAND( memtests, kshell_app_memtests_main )

extern kernel_physical_memory_next;
extern kernel_virtual_memory_next;

int kshell_app_memtests_main( int argc, char *argv[] ) {
	bool test_physical_limit = false;
	bool test_virtual_limit = false;
	bool full_test = false;

	if( argc == 2 ) {
		if( strcmp(argv[1], "phys") == 0 ) {
			test_physical_limit = true;
		} else if( strcmp(argv[1], "virt") == 0 ) {
			test_virtual_limit = true;
		} else {
			full_test = true;
		}
	} else {
		full_test = true;
	}

	if( test_physical_limit ) {
		debugf( "Testing physical memory limit.\n" );

		kernel_physical_memory_next = 0x00000000BF650000;
	
		void *page2 = page_allocate_kernel(1);
		debugf( "Test 1: virt=0x%016llX phys=0x%016llX.\n", page2, paging_virtual_to_physical(page2) );

		for( int i = 0; i < (0xbf56c000 / 4096) + 100; i++ ) {
			void *page = page_allocate_kernel(1);

			debugf( "Test %d: virt=0x%016llX phys=0x%016llX write=", i, page, paging_virtual_to_physical(page) );

			uint16_t *data = page;
			*data = i;

			debugf_raw( "%d write_virt=0x%016llX\n", *data, data );
		} 
	}

	if( test_virtual_limit ) {
		debugf( "Testing virtual memory limit.\n" );

		kernel_virtual_memory_next = 0xFFFFFFFFFFFE0000;

		for( int i = 0; i < (0xbf56c000 / 4096) + 100; i++ ) {
			void *page = page_allocate_kernel(1);

			debugf( "Test %d: virt=0x%016llX phys=0x%016llX write=", i, page, paging_virtual_to_physical(page) );

			uint16_t *data = page;
			*data = i;

			debugf_raw( "%d write_virt=0x%016llX\n", *data, data );
		} 
	}

	if( full_test ) {
		debugf( "Testing full.\n" );

		for( int i = 0; i < (0xbf56c000 / 4096); i++ ) {
			void *page = page_allocate_kernel(1);

			debugf( "Test %d: virt=0x%016llX phys=0x%016llX write=", i, page, paging_virtual_to_physical(page) );

			uint16_t *data = page;
			*data = i;

			debugf_raw( "%d write_virt=0x%016llX\n", *data, data );
		} 
	}
	
	return 0;
}