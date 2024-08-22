#include <kernel_common.h>
#include <kmemory.h>
#include <page_group.h>

/*
A page group is a logical and physical collection of contigous pages. Internally the group is represented by a bitmap, with one bit equal to one page.

	
 */
page_group *page_group_new( page_group *group ) {
	group = kmalloc( sizeof(page_group) );

	
}

bool page_group_setup_bitmap( page_group *group, uint64_t size ) {
	debugf( "pg:                   0x%016llX\n", group );
	debugf( "pg_page_size:         0x%016llX\n", group->page_size );

	group->num_pages = size / group->page_size;

	bool res = vi_bitmap_create( &group->page_bitmap, group->num_pages );

	if( !res ) {
		debugf( "Page group bitmap unable to be created.\n" );
		return false;
	}
}

/**
 * @brief Returns the address of the next free page
 * 
 * @param group 
 * @return uint64_t physical address of the next free page, 0 if no page found
 */
uint64_t page_group_get_next_free( page_group *group ) {
	for( uint32_t i = 0; i < group->num_pages; i++ ) {
		if( vi_bitmap_test( &group->page_bitmap, i ) == false ) {
			return PAGE_GROUP_BIT_TO_ADDR( group, i );
		}
	}

	return 0;
}

/**
 * @brief Returns the address of the next free page and allocates it
 * 
 * @param group 
 * @return uint64_t physical address of the next free page, 0 if no page found
 */
uint64_t page_group_allocate_next_free( page_group *group ) {
	uint64_t addr = page_group_get_next_free( group );

	if( addr == 0 ) {
		return 0;
	}

	vi_bitmap_set( &group->page_bitmap, PAGE_GROUP_ADDR_TO_BIT(group, addr) );

	return addr;
}

uint64_t page_group_get_page_block( page_group *group, uint32_t num_pages ) {
	if( group == NULL ) {
		return 0;
	}

	if( num_pages < 1 || num_pages > group->num_pages ) {
		return 0;
	}

	for( uint32_t i = 0; i < group->num_pages; i++ ) {
		if( vi_bitmap_test( &group->page_bitmap, i ) == false ) {
			// We found a potential block
			
			// if num_pages == 1, then just be done
			if( num_pages == 1 ) {
				return PAGE_GROUP_BIT_TO_ADDR( group, i );
			}

			// check num_pages additional pages
			uint32_t n = 0;
			for( n = i + 1; n < num_pages; n++ ) {
				// Don't go past the end of the page group
				if( n >= group->num_pages ) {
					i = n;
					n = 0;
					break;
				}

				if( vi_bitmap_test( &group->page_bitmap, n ) == false ) {
					// If we get here then we know we don't have enough pages,
					// set i to n so we can skip these
					i = n;
					n = 0;
					break;
				}
			}

			// if n is 0 (which it can't be normally), then we know we havne't found something, so bail out of this
			if( n == 0 ) {
				continue;
			}

			// If we get here then we have a group
			return PAGE_GROUP_BIT_TO_ADDR( group, i );
		}
	}

	// If we haven't returned already, we failed to find a group
	return 0;
}

bool page_group_is_free( page_group *group, uint64_t addr ) {
	// ensure the addr is valid at least
	if( addr < group->physical_base || addr > (group->physical_base + (group->num_pages * group->page_size)) ) {
		return false;
	}

	return vi_bitmap_test( &group->page_bitmap, PAGE_GROUP_ADDR_TO_BIT(group,addr) );
}

bool page_group_is_num_free( page_group *group, uint32_t number ) {
	return vi_bitmap_test( &group->page_bitmap, number );
}

void page_group_add_page( ) {

}

void page_group_delete( page_group *group ) {

}

void page_group_duplicate( page_group *new, page_group *old ) {

}