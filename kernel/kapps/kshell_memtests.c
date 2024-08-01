#include <kernel_common.h>
#include <kshell_app.h>
#include <page.h>

KSHELL_COMMAND( mem, kshell_app_mem_main )

extern uint64_t kernel_physical_memory_next;
extern uint64_t kernel_virtual_memory_next;
extern kinfo kernel_info;

bool test_physical_limit = false;
bool test_virtual_limit = false;
bool full_test = false;

void tests( void );
void stats( void );
void display_help( void );

int kshell_app_mem_main( int argc, char *argv[] ) {

	if( argc >= 2 ) {
		if( strcmp(argv[1], "test") == 0 ) {
			if( argc == 3 ) {
				if( strcmp(argv[1], "phys") == 0 ) {
					test_physical_limit = true;
					tests();
				} else if( strcmp(argv[1], "virt") == 0 ) {
					test_virtual_limit = true;
					tests();
				} else if( strcmp(argv[3], "full") == 0 ) {
					full_test = true;
					tests();
				} else {
					display_help();
				}
			} else {
				display_help();
			}
		} else if( strcmp(argv[1], "stats") == 0 ) {
			stats();
		}
		
	} else {
		display_help();
	}


	
	return 0;
}

void tests( void ) {
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
}

void display_help( void ) {
	printf( "mem : a memory diagnostic tool.\n" );
	printf( "    mem test <virt | phys | full>\n" );
	printf( "    mem stats\n" );
}

void stats( void ) {
	uint64_t virt_mem_base = kernel_info.kernel_end;
	uint64_t virt_mem_max = 0xFFFFFFFFFFFFF000;
	uint64_t virt_mem_used = kernel_virtual_memory_next - virt_mem_base;
	uint64_t virt_mem_total_size = virt_mem_max - virt_mem_base;
	//double virt_mem_percent_allocated = (1714388992 / virt_mem_total_size);

	uint64_t phys_mem_base = kernel_info.kernel_allocate_memory_start;
	uint64_t phys_mem_max = kernel_info.kernel_allocate_memory_start + kernel_info.kernel_allocate_memory_size;
	uint64_t phys_mem_total_size = kernel_info.kernel_allocate_memory_size;
	uint64_t phys_mem_used = kernel_physical_memory_next - phys_mem_base;

	uint64_t page_size = 4096;
	uint64_t pages_max = virt_mem_total_size/page_size;
	uint64_t pages_allocated = virt_mem_used/page_size;
	uint64_t pages_free = pages_max - pages_allocated;

	dpf( "Kernel:\n" );
	dpf( "    virt memory base:        0x%016llX\n", virt_mem_base );
	dpf( "    virt memory max:         0x%016llX\n", virt_mem_max );
	dpf( "    virt memory next:        0x%016llX\n", kernel_virtual_memory_next );
	dpf( "    virt memory total size:  0x%llX (%u)\n", virt_mem_total_size, virt_mem_total_size );
	dpf( "    virt memory used:        0x%llX (%u)\n", virt_mem_used, virt_mem_used );
	//dpf( "    virt memory %% allocated: %d%%\n", virt_mem_percent_allocated );
	dpf( "\n" );
	dpf( "    phys memory base:        0x%016llX\n", phys_mem_base );
	dpf( "    phys memory max:         0x%016llX\n", phys_mem_max );
	dpf( "    phys memory next:        0x%016llX\n", kernel_physical_memory_next );
	dpf( "    phys memory total size:  0x%llX (%u)\n", phys_mem_total_size, phys_mem_total_size );
	dpf( "    phys memory used:        0x%llX (%u)\n", phys_mem_used, phys_mem_used);
	//dpf( "    phys memory %% allocated: %d%%\n", phys_mem_used / phys_mem_total_size );
	dpf( "\n" );
	dpf( "    page size:               0x%X (%u)\n", 4096, 4096 );
	dpf( "    pages max:               0x%X (%u)\n", pages_max, pages_max );
	dpf( "    pages allocated:         0x%X (%u)\n", pages_allocated, pages_allocated );
	dpf( "    pages free:              0x%X (%u)\n", pages_free, pages_free );
	//dpf( "    pages %% allocated:       %d%%\n", pages_allocated / pages_max );
	dpf( "\n" );
}