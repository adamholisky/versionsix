#ifndef VIOS_PAGEGROUP_INCLUDED
#define VIOS_PAGEGROUP_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <page.h>
#include <lib/bitmap.h>

#define PAGE_GROUP_ADDR_TO_BIT( group, addr ) (addr - group->physical_base) / group->page_size
#define PAGE_GROUP_BIT_TO_ADDR( group, bit ) (bit * group->page_size) + group->physical_base

typedef struct {
	uint64_t physical_base;
	uint64_t virtual_base;

	struct pagegroup_page *prev;
	struct pagegroup_page *next;
} page_group_page;

typedef struct {
	vi_bitmap *page_bitmap;
	uint64_t physical_base;
	uint32_t num_pages;
	uint32_t page_size;

	struct page_group *parent;
	struct page_group *child;
	struct page_group *next;
} page_group;

page_group *page_group_new( page_group *group );
bool page_group_setup_bitmap( page_group *group, uint64_t size );

uint64_t page_group_get_next_free( page_group *group );
uint64_t page_group_allocate_next_free( page_group *group );

uint64_t page_group_get_page_block( page_group *group, uint32_t num_pages );
uint64_t page_group_allocate_page_block( page_group *group, uint32_t num_pages );

bool page_group_is_num_free( page_group *group, uint32_t number );
bool page_group_is_free( page_group *group, uint64_t addr );

#ifdef __cplusplus
}
#endif
#endif