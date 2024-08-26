#include "kernel_common.h"
#include <limine.h>
#include "page.h"
#include <page_group.h>
#include <string.h>

uint64_t kernel_physical_base;
uint64_t kernel_physical_allocation_base;
uint64_t kernel_virtual_base;
uint64_t kernel_virtual_allocation_base;
uint64_t kernel_heap_virtual_memory_next;
uint64_t kernel_heap_physical_memory_next;

// lol we support a max of 16gb memory now
uint64_t const max_memory = 0x240000000; // emulator runs at 8gb, so cheat for now
uint64_t const identity_map_start = 0xFFFF800000000000;
uint64_t identity_map_end = identity_map_start + max_memory;
uint64_t identity_map[512] __attribute__ ((aligned (4096)));

uint64_t k_pml4[512] __attribute__ ((aligned (4096)));
uint64_t k_pdpt[512] __attribute__ ((aligned (4096)));
uint64_t k_pd[512] __attribute__ ((aligned (4096)));
uint64_t k_pt1[512] __attribute__ ((aligned (4096)));
uint64_t k_pt2[512] __attribute__ ((aligned (4096)));
uint64_t k_pt3[512] __attribute__ ((aligned (4096)));
uint64_t k_pt4[512] __attribute__ ((aligned (4096)));

uint16_t k_current_pml4_index;
uint16_t k_current_pdpt_index;
uint16_t k_current_pd_index;
uint16_t k_current_pt_index;

uint64_t *k_current_pml4;
uint64_t *k_current_pdpt;
uint64_t *k_current_pd;
uint64_t *k_current_pt;

uint64_t k_remaining_pd;
uint64_t k_remaining_pt;

uint64_t kernel_virtual_memory_next;
uint64_t kernel_physical_memory_next;

uint64_t *k_new_pt;
uint64_t *k_new_pd;

extern kinfo kernel_info;

// Intentionally turning off warnings given we need to smash data together
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#pragma GCC diagnostic ignored "-Wint-conversion"

uint64_t magic = 0xBAD011112222B000;

#undef KDEBUG_PAGING_INIT_SETUP
void paging_setup_initial_structures( void ) {
	#ifdef KDEBUG_PAGING_INIT_SETUP
	// Sanity check myself
	uint64_t *magic_2 = (uint64_t)&magic - kernel_info.kernel_virtual_base + kernel_info.kernel_physical_base + kernel_info.hhdm_offset;
	debugf( "magic_2:  0x%016llX\n", magic_2 );
	debugf( "*magic_2: 0x%016llX\n", *magic_2 );
	#endif

	// Setup the identity map
	uint64_t cr3_contents = get_cr3();
	uint64_t *pml4 = cr3_contents + kernel_info.hhdm_offset;
	
	#ifdef KDEBUG_PAGING_INIT_SETUP
	debugf( "cr3 contents: 0x%016llX\n", cr3_contents );
	debugf( "pml4: 0x%016llX\n", pml4 );
	debugf( "*pml4:0x%016llX\n", *pml4 );

	
	paging_get_indexes( identity_map_start, &im_page_index );

	debugf( "start indexes: pml4: %llx    pdpt: %x    pd: %x    pt: %x\n", im_page_index.pml4, im_page_index.pdpt, im_page_index.pd, im_page_index.pt );
	#endif

	page_indexes im_page_index;
	paging_get_indexes( identity_map_end, &im_page_index );
	
	#ifdef KDEBUG_PAGING_INIT_SETUP
	debugf( "end indexes:   pml4: %x    pdpt: %x    pd: %x    pt: %x\n", im_page_index.pml4, im_page_index.pdpt, im_page_index.pd, im_page_index.pt );
	#endif

	uint64_t im_phys_addr = identity_map;
	im_phys_addr = im_phys_addr - kernel_info.kernel_virtual_base + kernel_info.kernel_physical_base;
	
	//debugf( "im_phys_addr: 0x%016llX\n", im_phys_addr );

	for( int i = 0; i < 8; i++ ) {
		identity_map[i] = paging_make_page( i * 0x40000000UL , PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE | PAGE_FLAG_PAGE_SIZE );
	}

	pml4[im_page_index.pml4] = paging_make_page( im_phys_addr, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );
	
	asm_refresh_cr3();

	//paging_diagnostic_cr3( cr3_contents );

	memset( k_pml4, 0, sizeof(uint64_t) * 512 );
	memset( k_pdpt, 0, sizeof(uint64_t) * 512 );
	memset( k_pd, 0, sizeof(uint64_t) * 512 );
	memset( k_pt1, 0, sizeof(uint64_t) * 512 );
	memset( k_pt2, 0, sizeof(uint64_t) * 512 );

	memmap_entry *kernel_memmap_entry = NULL;

	uint64_t k_page_size = 0x1000;

	for( int i = 0; i < kernel_info.memmap_count; i++ ) {
		if( kernel_info.memmap[i].type == LIMINE_MEMMAP_KERNEL_AND_MODULES ) {
			kernel_memmap_entry = &kernel_info.memmap[i];
		}
	}

	if( kernel_memmap_entry == NULL ) {
		debugf( "Could not find kernel memory map entry.\n" );
		do_immediate_shutdown();
	}

	uint16_t num_kernel_pages = kernel_memmap_entry->size / k_page_size;
	if( kernel_memmap_entry->size % k_page_size ) {
		num_kernel_pages++;
	}

	uint64_t kpages_current_physical = 0xFFFFFFFF80000000 - kernel_info.kernel_virtual_base + kernel_info.kernel_physical_base;

	#ifdef KDEBUG_PAGING_INIT_SETUP
	debugf( "num_kernel_pages: 0x%X\n", num_kernel_pages );
	#endif

	k_current_pml4 = k_pml4;
	k_current_pdpt = k_pdpt;
	k_current_pd = k_pd;

	for( uint16_t i = 0; i < num_kernel_pages; i++ ) {
		page_indexes index;
		paging_get_indexes( 0xFFFFFFFF80000000 + (i * k_page_size), &index );

		#ifdef KDEBUG_PAGING_INIT_SETUP
		debugf( "For: 0x%016llX\n", kpages_current_physical );
		debugf( " indexes: pml4: %llx    pdpt: %x    pd: %x    pt: %x\n", index.pml4, index.pdpt, index.pd, index.pt );
		#endif

		//debugf( " indexes: pml4: %llx    pdpt: %x    pd: %x    pt: %x\n", index.pml4, index.pdpt, index.pd, index.pt );

		uint64_t k_pt_physical = 0;

		if( index.pd == 0 ) {
			k_current_pt = k_pt1;
			k_pt1[index.pt] = paging_make_page( kpages_current_physical, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );
			k_pt_physical = (uint64_t)(&k_pt1[index.pt]) - kernel_info.kernel_virtual_base + kernel_info.kernel_physical_base;
		} else if( index.pd == 1 ) {
			k_current_pt = k_pt2;
			k_pt2[index.pt] = paging_make_page( kpages_current_physical, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );
			k_pt_physical = (uint64_t)(&k_pt2[index.pt]) - kernel_info.kernel_virtual_base + kernel_info.kernel_physical_base;
		} else if( index.pd == 2 ) {
			k_current_pt = k_pt3;
			k_pt3[index.pt] = paging_make_page( kpages_current_physical, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );
			k_pt_physical = (uint64_t)(&k_pt3[index.pt]) - kernel_info.kernel_virtual_base + kernel_info.kernel_physical_base;
		} else if( index.pd == 3 ) {
			k_current_pt = k_pt4;
			k_pt4[index.pt] = paging_make_page( kpages_current_physical, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );
			k_pt_physical = (uint64_t)(&k_pt4[index.pt]) - kernel_info.kernel_virtual_base + kernel_info.kernel_physical_base;
		} else {
			debugf( "Ran out of PDs for kernel. Time to refactor this shit.\n" );
			do_immediate_shutdown();
		}

		#ifdef KDEBUG_PAGING_INIT_SETUP
		debugf( "kpages_current_phys: 0x%016llX\n", kpages_current_physical );
		#endif
		
		if( !GET_PDE_PRESENT(k_pd[index.pd]) ) {
			//debugf( "***** CREATING PD: index.pd: %X\n", index.pd );
			k_pd[index.pd] = paging_make_page( k_pt_physical, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );
		}

		if( !GET_PDE_PRESENT(k_pdpt[index.pdpt]) ) {
			//debugf( "***** CREATING PDPT: index.pdpt: %X\n", index.pdpt );
			k_pdpt[index.pdpt] = paging_make_page( (uint64_t)(&k_pd[index.pd]) - kernel_info.kernel_virtual_base + kernel_info.kernel_physical_base, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );
		}

		if( !GET_PDE_PRESENT(k_pml4[index.pml4]) ) {
			//debugf( "***** CREATING PML4: index.pml4: %X\n", index.pml4 );
			k_pml4[index.pml4] = paging_make_page( (uint64_t)(&k_pdpt[index.pdpt]) - kernel_info.kernel_virtual_base + kernel_info.kernel_physical_base, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );
		}

		kpages_current_physical = kpages_current_physical + k_page_size;
	}

	k_pml4[im_page_index.pml4] = paging_make_page( im_phys_addr, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );

	#ifdef KDEBUG_PAGING_INIT_SETUP
	debugf( "k_PDPT: 0x%llX\n", k_pdpt );
	#endif
	

	uint64_t k_pml4_phys = (uint64_t)&k_pml4 - kernel_info.kernel_virtual_base + kernel_info.kernel_physical_base;

	#ifdef KDEBUG_PAGING_INIT_SETUP
	debugf( "k_pml4_virt: 0x%016llX\n", k_pml4 );
	debugf( "k_pml4_phys: 0x%016llX\n", k_pml4_phys );
	
	paging_diagnostic_cr3( k_pml4_phys );

	debugf( "Hit it.\n" );
	#endif

	set_cr3( k_pml4_phys );

	

	#ifdef KDEBUG_PAGING_INIT_SETUP
	debugf( "Done.\n" );
	#endif
}

#undef DEBUG_PAGING_INITALIZE
void paging_initalize( void ) {
	uint64_t cr3 = get_cr3();
	paging_cr3 *paging_cr3_data = (paging_cr3 *)&cr3;

	kernel_virtual_base = kernel_info.kernel_virtual_base;
	kernel_physical_base = kernel_info.kernel_physical_base;

	kernel_virtual_allocation_base = kernel_info.kernel_end;
	kernel_physical_allocation_base = kernel_info.kernel_allocate_memory_start;
	
	kernel_virtual_memory_next = kernel_info.kernel_end;
	kernel_physical_memory_next = kernel_info.kernel_allocate_memory_start;

	kernel_heap_virtual_memory_next = 0x0000700000000000;
	kernel_heap_physical_memory_next = kernel_info.usable_memory_start;

	page_indexes indexes;
	paging_get_indexes( kernel_virtual_memory_next, &indexes );
	k_current_pml4_index = indexes.pml4;
	k_current_pdpt_index = indexes.pdpt;
	k_current_pd_index = indexes.pd;
	k_current_pt_index = indexes.pt;
	
	k_new_pt = 0xFFFF000000000000;
	k_new_pd = 0xFFFF000000000000;

	k_new_pt = page_allocate_kernel_linear(1);
	k_new_pd = page_allocate_kernel_linear(1);
	
	debugf( "k_new_pt: 0x%016llX\n", k_new_pt );
	debugf( "k_new_pd: 0x%016llX\n", k_new_pd );

	/* 
	
	// This works, leaving for a bit
	
	for( int i = 0; i < 2000; i++ ) {
		uint16_t pd_idx_save = k_current_pd_index;
		uint16_t pt_idx_save = k_current_pt_index;
		uint64_t *p = page_allocate_kernel_linear(1);
		debugf( "%d  p: 0x%016llX    pd: %X    pt: %X\n", i, p, pd_idx_save, pt_idx_save );
	} 
	*/

	/* void *test_code_page = page_allocate_kernel_linear(1);
	void *test_data_page = page_allocate_kernel_linear(1);

	uint8_t *virt_cp = 0;
	uint8_t *virt_dp = 0x200000;

	paging_page_map_to_pml4( k_pml4, test_code_page - kernel_virtual_allocation_base + kernel_physical_allocation_base, virt_cp, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );

	virt_cp[0] = 'A';
	virt_cp[1] = 'H';

	debugf( "virt_cp: %s\n", virt_cp );

	paging_page_map_to_pml4( k_pml4, test_data_page - kernel_virtual_allocation_base + kernel_physical_allocation_base, virt_dp, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );

	virt_dp[0] = 'K';
	virt_dp[1] = 'S';

	debugf( "virt_dp: %s\n", virt_dp ); */


	// Let's do a paging test, fail hard if it fails
	kernel_info.in_paging_sanity_test = true;
	uint64_t *paging_sanity_check = page_allocate_kernel(1);
	*paging_sanity_check = 0x00001111BADAB000;
	if( *paging_sanity_check != 0x00001111BADAB000 ) {
		debugf( "Paging sanity check: fail\n" );
		do_immediate_shutdown();
	} else {
		debugf( "Paging sanity check: pass\n" );
	}

	uint64_t *paging_sanity_check_2 = page_map( 0x00001122BADA0000, 0x0000000090000000 );

	*paging_sanity_check_2 = 30;
	*(paging_sanity_check_2 + 1) = 40;
	
	if( *paging_sanity_check_2 != 30 ) {
		debugf( "Paging sanity check 2: fail\n" );
		do_immediate_shutdown();
	} else {
		debugf( "Paging sanity check 2: pass\n" );
	}

	kernel_info.in_paging_sanity_test = false;

	#ifdef DEBUG_PAGING_INITALIZE
	debugf( "limine_virt_mem_next: 0x%llx\n", kernel_virtual_memory_next );
	debugf( "limine_phys_mem_next: 0x%llx\n", kernel_physical_memory_next );

	debugf( "Allocating 1 page.\n" );
	uint64_t *page_test = page_allocate( 1 );

	debugf( "limine_virt_mem_next: 0x%llx\n", kernel_virtual_memory_next );
	debugf( "limine_phys_mem_next: 0x%llx\n", kernel_physical_memory_next );

	debugf( "R/W Test ...\n" );

	*page_test = 10;
	*(page_test + 1) = 20;

	debugf( "*page_test: %d\n", *page_test );
	debugf( "*(page_test + 1): %d\n", *(page_test + 1) );

	// Test full page map creation
	uint64_t *page_test_2 = page_map( 0x00001122BADA0000, 0x0000000090000000 );

	*page_test_2 = 30;
	*(page_test_2 + 1) = 40;

	debugf( "page_test_2: %llx\n", (uint64_t)page_test_2 );
	debugf( "*page_test_2: %d\n", *page_test_2 );
	debugf( "*(page_test_2 + 1): %d\n", *(page_test_2 + 1) );
	#endif
}

page_group main_page_group;
bool use_page_group = false;

void paging_initalize_page_groups( void ) {
	main_page_group.physical_base = kernel_info.usable_memory_start;
	main_page_group.page_size = PAGE_SIZE;

	// Start from the base of the allocatable memory
	page_group_setup_bitmap( &main_page_group, kernel_info.usable_memory_size );

	// Post setup, we should have the total allocated
	uint64_t pages_already_allocated = (kernel_heap_physical_memory_next - kernel_info.usable_memory_start) / PAGE_SIZE;

	uint64_t end_of_allocation_sync = 0;

	for( int i = 0; i < pages_already_allocated; i++ ) {
		end_of_allocation_sync = page_group_allocate_next_free( &main_page_group );
		debugf( "i: %d\n", i );
	}

	if( end_of_allocation_sync + 0x1000 != kernel_heap_physical_memory_next ) {
		debugf( "WARNING! Page group allocation sync failed.\n" );
	} else {
		use_page_group = true;
	}
}

void paging_diagnostic_cr3( uint64_t cr3_physical ) {
	//debugf( "cr3_physical: 0x%016llX\n", cr3_physical );

	uint64_t *pml4 = cr3_physical + kernel_info.hhdm_offset;
	//debugf( "pml4 == 0x%llX\n", pml4 );

	for( uint16_t i = 0; i < 512; i++ ) {
		if( GET_PDE_PRESENT(pml4[i]) ) {
			paging_diagnostic_output_entry( pml4[i], 0x0, i, "", "PML4" );

			//debugf( "pml4[0x%02X]: 0x%016llx\n", i, pml4[i] );

			uint64_t pdpt_addr = (pml4[i] & 0x000FFFFFFFFFF000);
			uint64_t *pdpt = pdpt_addr + kernel_info.hhdm_offset;

			//debugf( "pdpt_addr: 0x%016llX\n", pdpt_addr );
			//debugf( "pdpt:      0x%016llX\n", pdpt );
			
			for( uint16_t j = 0; j < 512; j++ ) {
				//debugf( "PDPT: 0x%llX\n", *pdpt );
				
				if( GET_PDE_PRESENT(pdpt[j]) ) {
					//db1();
					paging_diagnostic_output_entry( pdpt[j], 0x0, j, "  ", "PDPT" );

					if( GET_PDE_PAGE_SIZE(pdpt[j]) == 0 ) {
						uint64_t pd_addr = (pdpt[j] & 0x000FFFFFFFFFF000);
						uint64_t *pd = (uint64_t *)(pd_addr + kernel_info.hhdm_offset);

						for( uint16_t k = 0; k < 512; k++ ) {
							if( GET_PDE_PRESENT(pd[k]) ) {
								paging_diagnostic_output_entry( pd[k], 0x0, k, "    ", "PD" );
								
								if( GET_PDE_PAGE_SIZE(pd[k]) == 0 ) {
									uint64_t pt_addr = (pd[k] & 0x000FFFFFFFFFF000);
									uint64_t *pt = (uint64_t *)(pt_addr + kernel_info.hhdm_offset);

									uint16_t pts_present = 0;
									for( uint16_t z = 0; z < 512; z++ ) {
										
										if( GET_PDE_PRESENT(pt[z]) ) {
											paging_diagnostic_output_entry( pt[z], 0x0, z, "      ", "PT" );
											pts_present++;
										}
									}
									debugf_raw( "      PT[x]: %d page tables present.\n", pts_present );
								}
							}
						}
					}
					
				}
			}
		}
	}
}

void paging_diagnostic_output_entry( uint64_t pg_dir_entry, uint64_t starting_virtual, uint16_t i, char *spaces, char *type ) {
	//debugf( "p_diag_out_entry  pg_dir_entry: 0x%llX   start_virt: 0x%llX  i: 0x%llX\n", pg_dir_entry, starting_virtual, i );
	
	char p = ( GET_PDE_PRESENT(pg_dir_entry) == 1 ? 'P' : ' ' );
	char rw = ( GET_PDE_READ_WRITE(pg_dir_entry) ? 'W' : 'R' );
	char cd = ( GET_PDE_CACHE_DISABLED(pg_dir_entry) ? 'C' : ' ' );
	char ps = ( GET_PDE_PAGE_SIZE(pg_dir_entry) ? 'S' : ' ' );

	if( p == 'P' ) {
		debugf_raw( "%s%s[0x%03X]: %c %c %c %c 0x%016llX\n", spaces, type, i, p, rw, cd, ps, pg_dir_entry );
	}
}

uint64_t paging_make_page( uint64_t physical_address, uint32_t flags ) {
	uint64_t page = 0;

	//page = SET_PDE_ADDRESS( page, physical_address );

	page = physical_address & 0x000FFFFFFFFFF000;

	if( flags & PAGE_FLAG_PRESENT ) { page = SET_PDE_PRESENT(page); }
	if( flags & PAGE_FLAG_READ_WRITE ) { page = SET_PDE_READ_WRITE(page); }
	if( flags & PAGE_FLAG_USER_SUPERVISOR ) { page = SET_PDE_USER_SUPERVISOR(page); }
	if( flags & PAGE_FLAG_WRITE_THROUGH ) { page = SET_PDE_WRITE_THROUGH(page); }
	if( flags & PAGE_FLAG_CACHE_DISABLED ) { page = SET_PDE_CACHE_DISABLED(page); }
	if( flags & PAGE_FLAG_ACCESSED ) { page = SET_PDE_ACCESSED(page); }
	if( flags & PAGE_FLAG_PAGE_SIZE ) { page = SET_PDE_PAGE_SIZE(page); }
	if( flags & PAGE_FLAG_EXECUTE_DIABLED ) { page = SET_PDE_EXECUTE_DISABLED(page); }

	//debugf( "Page made: 0x%016llX\n", page );

	return page;
}

void paging_get_indexes( uint64_t virtual_address, page_indexes *indexes ) {
	indexes->pml4 = (virtual_address >> 39) & 0x1FF;
	indexes->pdpt = (virtual_address >> 30) & 0x1FF;
	indexes->pd = (virtual_address >> 21) & 0x1FF;
	indexes->pt = (virtual_address >> 12) & 0x1FF;
}

uint64_t paging_get_addr_from_index( uint16_t index_pml4, uint16_t index_pdpt, uint16_t index_pd, uint16_t index_pt ) {
	uint64_t addr = 0;

	addr = addr | ((uint64_t)index_pml4 << 39 );
	addr = addr | ((uint64_t)index_pdpt << 30 );
	addr = addr | ((uint64_t)index_pd << 21 );
	addr = addr | ((uint64_t)index_pt << 12 );
}



#undef KDEBUG_PAGE_MAP_PML4
bool KDEBUG_PAGE_MAP_ACTIVE = false;

uint64_t paging_page_map_to_pml4( uint64_t *pml_4, uint64_t physical_address, uint64_t virtual_address, uint64_t flags ) {
	#ifdef KDEBUG_PAGE_MAP_PML4
	KDEBUG_PAGE_MAP_ACTIVE = true;
	#endif

	if( KDEBUG_PAGE_MAP_ACTIVE ) {
		debugf( "pml_4: 0x%016llX  phys_addr: 0x%016llX  virt_addr: 0x%016llX  flags: 0x%016llX\n", pml_4, physical_address, virtual_address, flags );
	}

	page_indexes virt_indexes;
	paging_get_indexes( virtual_address, &virt_indexes );

	bool setup_pml4 = false;
	bool setup_pdpt = false;
	bool setup_pd = false;
	bool setup_pt = false;

	uint64_t *pml4 = NULL;
	uint64_t *pdpt = NULL;
	uint64_t *pd = NULL;
	uint64_t *pt = NULL;

	pml4 = pml_4;

	if( pml_4 == NULL ) {
		pml4 = get_cr3() + ADDR_IDENTITY_MAP;
	}

	if( KDEBUG_PAGE_MAP_ACTIVE ) {
		debugf( "pml4: 0x%016llX\n", pml4 );
	}

	if( !(pml4[ virt_indexes.pml4 ] & PAGE_FLAG_PRESENT) ) {
		// Do we have the pml4 entry setup for the pdpt? No, then set it up.
		setup_pdpt = true;
		setup_pd = true;
		setup_pt = true;
	} else {
		pdpt = (pml4[ virt_indexes.pml4 ] & PAGE_ADDR_MASK) + ADDR_IDENTITY_MAP;
		
		if( !(pdpt[ virt_indexes.pdpt ] & PAGE_FLAG_PRESENT) ) {
			// Do we have the pdpt entry setup for the pd? No, then set it up
			setup_pd = true;
			setup_pt = true;
		} else {
			pd = (pdpt[ virt_indexes.pdpt ] & PAGE_ADDR_MASK ) + ADDR_IDENTITY_MAP;

			if( !(pd[ virt_indexes.pd ] & PAGE_FLAG_PRESENT ) ) {
				// Do we have the pd entry set up for the pt? No, then set it up
				setup_pt = true;
			} else {
				pt = (pd[ virt_indexes.pd ] & PAGE_ADDR_MASK ) + ADDR_IDENTITY_MAP;

				if( pt == 0 ) {
					setup_pt = true;
				}
			}
		}
	}

	if( KDEBUG_PAGE_MAP_ACTIVE ) {
		debugf( "Pre-setup pdpt: 0x%016llx\n", pdpt );
		debugf( "Pre-setup pd:   0x%016llx\n", pd );
		debugf( "Pre-setup pt:   0x%016llx\n", pt );
		debugf( "setup_pml4: %d @ index 0x%X\n", setup_pml4, virt_indexes.pml4 );
		debugf( "setup_pdpt: %d @ index 0x%X\n", setup_pdpt, virt_indexes.pdpt );
		debugf( "setup_pd: %d @ index 0x%X\n", setup_pd, virt_indexes.pd );
		debugf( "setup_pt: %d @ index 0x%X\n", setup_pt, virt_indexes.pt );
	}

	if( setup_pdpt ) {
		pdpt = (uint64_t *)page_allocate_kernel_linear(1);

		if( KDEBUG_PAGE_MAP_ACTIVE ) {
			debugf( "pdpt: 0x%016llX\n", pdpt );
			debugf( "kvab: 0x%016llX\n", kernel_virtual_allocation_base );
			debugf( "pdpt: 0x%016llX\n", (void *)pdpt - kernel_virtual_allocation_base );
			debugf( "pdpt: 0x%016llX\n", (void *)pdpt - kernel_virtual_allocation_base + kernel_physical_allocation_base );
		}

		pml4[virt_indexes.pml4] = paging_make_page( (void *)pdpt - kernel_virtual_allocation_base + kernel_physical_allocation_base, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );

		if( KDEBUG_PAGE_MAP_ACTIVE ) {
			debugf( "allocated pdpt: %llx\n", pdpt );
		}
	}

	if( setup_pd ) {
		pd = (uint64_t *)page_allocate_kernel_linear(1);
		pdpt[virt_indexes.pdpt] = paging_make_page( (void *)pd - kernel_virtual_allocation_base + kernel_physical_allocation_base, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );

		if( KDEBUG_PAGE_MAP_ACTIVE ) {
			debugf( "allocated pd: %llx\n", pd );
		}
	}

	if( setup_pt ) {
		pt = (uint64_t *)page_allocate_kernel_linear(1);
		pd[virt_indexes.pd] = paging_make_page( (void *)pt - kernel_virtual_allocation_base + kernel_physical_allocation_base, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );
		
		if( KDEBUG_PAGE_MAP_ACTIVE ) {
			debugf( "allocated pt: %llx\n", pt );
		}
	}

	pt[virt_indexes.pt] = paging_make_page( physical_address, flags );
	paging_invalidate_page( virtual_address );
	
	return virtual_address;
}

/**
 * @brief Allocates the next available kernel page.
 * 
 * @param number_of_pages 
 * @return uint64_t* 
 */
void *page_allocate_kernel_linear( uint32_t number_of_pages ) {
	__asm__ volatile("cli");
	uint64_t return_virt_addr = NULL;

	if( number_of_pages < 1 ) {
		return NULL;
	}

	// We want to make sure the new_pt and new_pd spaces are available 
	// BEFORE we start to allocate pages for the call. Otherwise the
	// addresses won't be linear.

	if( k_current_pd == k_new_pd ) {
		//debugf( "In new PD.\n" );
		k_new_pd = paging_allocate_single_linear_kernel_page();
		paging_increment_kernel_page_index();
	}

	if( k_current_pt == k_new_pt ) {
		//debugf( "In new PT.\n" );
		k_new_pt = paging_allocate_single_linear_kernel_page();
		paging_increment_kernel_page_index();
	}

	// This needs to happen here because the above commands might alter k_vm_next
	return_virt_addr = kernel_virtual_memory_next;

	// Finally increment through the needed pages
	for( int i = 0; i < number_of_pages; i++ ) {
		paging_allocate_single_linear_kernel_page();
		paging_increment_kernel_page_index();
	}

	__asm__ volatile("sti");
	return return_virt_addr;
}

/**
 * @brief Allocates a single linear page in the kernel space
 * 
 * BIG THING -- we can't increment indexes here because this is used to allocate
 * a new page for the next PT or PD. Setting the k_new_pt and k_new_pd happens
 * after this function returns.
 * 
 * @return void* 
 */
void *paging_allocate_single_linear_kernel_page( void ) {
	//debugf( "virt: 0x%016llX    phys: 0x%016llX\n", kernel_virtual_memory_next, kernel_physical_memory_next );

	page_indexes idx;
	paging_get_indexes( kernel_virtual_memory_next, &idx );

	// Create the page
	uint64_t page_entry = paging_make_page( kernel_physical_memory_next, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );


	// Set the entry in the page table
	k_current_pt[k_current_pt_index] = page_entry;
	paging_invalidate_page( kernel_virtual_memory_next );

	// Zero out the page
	memset( (uint8_t *)kernel_virtual_memory_next, 0, PAGE_SIZE );

	// Move the pointers forward one page
	kernel_virtual_memory_next = kernel_virtual_memory_next + PAGE_SIZE;
	kernel_physical_memory_next = kernel_physical_memory_next + PAGE_SIZE;

	return kernel_virtual_memory_next - PAGE_SIZE;
}

/**
 * @brief Increments the indexes in the kernel page handlers.
 * 
 * At the end of this function, the following are guarnateed to be valid:
 * 1. k_current_pt_index
 * 2. k_curernt_pt
 * 3. k_current_pd_index
 * 4. k_current_pd
 * 
 */
void paging_increment_kernel_page_index( void ) {
	// If we allocated PT entry #511, then roll over to a new PT
	// else increment the index
	if( k_current_pt_index == 511 ) {
		k_current_pt_index = 0;
		k_current_pt = k_new_pt;

		debugf( "k_new_pt: 0x%016llX\n", k_new_pt );
		debugf( "k_new_pt: 0x%016llX\n", (void *)k_new_pt - kernel_virtual_allocation_base );
		debugf( "k_new_pt: 0x%016llX\n", (void *)k_new_pt - kernel_virtual_allocation_base + kernel_physical_allocation_base );


		k_current_pd[k_current_pd_index + 1] = paging_make_page( (void *)k_new_pt - kernel_virtual_allocation_base + kernel_physical_allocation_base, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );
		paging_invalidate_page( k_new_pt );

		k_current_pd_index++;
	} else {
		k_current_pt_index++;
	}

	// If we allocated (above) PD index 512, then roll over to a new PD
	// else do nothing.
	// This is 512 because 511 is a valid PD entry, and 512 would be hit
	// when the pd is incremented above.
	if( k_current_pd_index == 512 ) {
		k_current_pd_index = 0;
		k_current_pd = k_new_pd;

		k_current_pdpt[k_current_pdpt_index] = paging_make_page( k_new_pd - kernel_virtual_base + kernel_physical_base, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );
		paging_invalidate_page( k_new_pd );

		k_current_pdpt_index++;
	}
}

void paging_use_next_kernel_page_index( void ) {
	
}

/**
 * @brief Invalidates the page.
 * 
 * @todo Write this soon
 * 
 * @param page_physical_address 
 */
void paging_invalidate_page( uint64_t page_virtual_address ) {
	asm_refresh_cr3();
}

/* OLD CODE */

#undef DEBUG_PAGE_ALLOCATE
void *page_allocate( uint32_t number ) {
	return page_allocate_provider( NULL, kernel_heap_virtual_memory_next, number, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE, false );
}

void *page_allocate_provider( uint64_t *pml4, uint64_t virt_mem_start, uint32_t number, uint64_t flags, bool contiguous ) {
	void *return_val = virt_mem_start;

	#ifdef DEBUG_PAGE_ALLOCATE
	log_entry_enter();
	debugf( "number: %d\n", number);
	#endif

	if( number < 1 ) {
		return NULL;
	}

	//if( use_page_group ) {
	if( false ) {
		if( contiguous ) {

		} else {
			uint64_t pg_addr = 0;

			for( int i = 0; i < number; i++ ) {
				pg_addr = page_group_allocate_next_free( &main_page_group );

				paging_page_map_to_pml4( pml4, pg_addr, virt_mem_start, flags );

				if( i == 0 ) {
					return_val = pg_addr;
				}
			}
			
		}
		
	} else {
		uint64_t vmem_current = virt_mem_start;

		for( int i = 0; i < number; i++ ) {
			paging_page_map_to_pml4( pml4, kernel_heap_physical_memory_next, vmem_current, flags );   
		
			vmem_current = vmem_current + PAGE_SIZE;
			kernel_heap_virtual_memory_next = kernel_heap_virtual_memory_next + PAGE_SIZE;
			kernel_heap_physical_memory_next = kernel_heap_physical_memory_next + PAGE_SIZE;
		}
	}

	#ifdef DEBUG_PAGE_ALLOCATE
	debugf( "Page allocated: 0x%016llX\n", return_val );
	log_entry_exit();
	#endif

	return return_val;
}

void *page_allocate_kernel( uint32_t number ) {
	//debugf( "number: %d\n", number );
	return page_allocate_kernel_linear( number );
}

void *page_allocate_kernel_mmio( uint32_t number ) {
	void *return_val =  NULL;

	#ifdef DEBUG_PAGE_ALLOCATE
	log_entry_enter();
	#endif

	if( number < 1 ) {
		return NULL;
	}

	// This makes sure we have consecutive linear pages for mmio use,
	// drivers generally like and need this.
	if( k_current_pt_index + number > 511 ) {
		do {
			paging_increment_kernel_page_index();
			debugf( "k_current_pt_index incremented.\n" );
		} while( k_current_pt_index != 0 );
	}

	return_val = kernel_virtual_memory_next;

	for( int i = 0; i < number; i++ ) {
		paging_page_map_to_pml4( NULL, kernel_physical_memory_next, kernel_virtual_memory_next,  PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE | PAGE_FLAG_CACHE_DISABLED | PAGE_FLAG_WRITE_THROUGH );   
		
		kernel_virtual_memory_next = kernel_virtual_memory_next + PAGE_SIZE;
		paging_increment_kernel_page_index();
		kernel_physical_memory_next = kernel_physical_memory_next + PAGE_SIZE;
	}

	#ifdef DEBUG_PAGE_ALLOCATE
	log_entry_exit();
	#endif

	//debugf( "Page allocated: 0x%016llX\n", return_val );
	return return_val;
}

void *page_allocate_mmio( uint32_t number ) {
	void *return_val = kernel_heap_virtual_memory_next;

	#ifdef DEBUG_PAGE_ALLOCATE
	log_entry_enter();
	#endif

	if( number < 1 ) {
		return NULL;
	}

	for( int i = 0; i < number; i++ ) {
		paging_page_map_to_pml4( NULL, kernel_heap_physical_memory_next, kernel_heap_virtual_memory_next, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE | PAGE_FLAG_CACHE_DISABLED | PAGE_FLAG_WRITE_THROUGH );   
		
		kernel_heap_virtual_memory_next = kernel_heap_virtual_memory_next + PAGE_SIZE;
		kernel_heap_physical_memory_next = kernel_heap_physical_memory_next + PAGE_SIZE;
	}

	#ifdef DEBUG_PAGE_ALLOCATE
	log_entry_exit();
	#endif

	debugf( "Page allocated: 0x%016llX\n", return_val );
	return return_val;
}

void *page_map( uint64_t virtual_address, uint64_t physical_address ) {
	return paging_page_map_to_pml4( NULL, physical_address, virtual_address, PAGE_FLAG_PRESENT | PAGE_FLAG_READ_WRITE );
}

void paging_dump_page_direct( uint64_t page ) {
	debugf( "Page Entry: 0x%016llx\n", page );
	debugf( "    PR: %d\n", GET_PDE_PRESENT( page ));
	debugf( "    RW: %d\n", GET_PDE_READ_WRITE( page ));
	debugf( "    US: %d\n", GET_PDE_USER_SUPERVISOR( page ));
	debugf( "    WT: %d\n", GET_PDE_WRITE_THROUGH( page ));
	debugf( "    CD: %d\n", GET_PDE_CACHE_DISABLED( page ));
	debugf( "    A:  %d\n", GET_PDE_ACCESSED( page ));
	debugf( "    PS: %d\n", GET_PDE_PAGE_SIZE( page ));
	debugf( "    ED: %d\n", GET_PDE_EXECUTE_DISABLED( page ));
	debugf( "    Address: 0x%08llx\n", GET_PDE_ADDRESS( page ));
}

void paging_dump_cr3( paging_cr3 *cr3 ) {
	debugf( "CR3 Paging Structure:\n" );
	debugf_raw( "    Reserved: %d\n", cr3->reserved );
	debugf_raw( "    Write Through: %d\n", cr3->write_through );
	debugf_raw( "    Cache Disabled: %d\n", cr3->cache_disabled );
	debugf_raw( "    ignored: %d\n", cr3->ignored );
	debugf_raw( "    address: 0x%llx (4kb aligned)\n", cr3->address );
}

void paging_dump_page( paging_page_entry *page ) {
	debugf( "PD Entry @ 0x%p:\n", page );
	debugf_raw( "    Present: %d\n", page->present );
	debugf_raw( "    RW: %d\n", page->rw );
	debugf_raw( "    User_supervisor: %d\n", page->user_supervisor );
	debugf_raw( "    Write through: %d\n", page->write_through );
	debugf_raw( "    Cache disabled: %d\n", page->cache_disabled );
	debugf_raw( "    Accessed: %d\n", page->accessed );

	debugf_raw( "    Ignored_1: %d\n", page->ignored_1 );
	debugf_raw( "    Page Size: %d\n", page->page_size );
	debugf_raw( "    Ignored_2: %d\n", page->ignored_2 );

	debugf_raw( "    Address: 0x%llx\n", page->address );

	debugf_raw( "    Reserved_1: %d\n", page->reserved_1 );
	debugf_raw( "    Available_2: %d\n", page->available_2 );

	debugf_raw( "    Execute Disable: %d\n", page->execute_disable );
}

void paging_examine_page_for_address( uint64_t virtual_address ) {
	/* uint64_t index_pml4 = (virtual_address >> 39) & 0x1FF;
	uint64_t index_pdpt = (virtual_address >> 30) & 0x1FF;
	uint64_t index_pd = (virtual_address >> 21) & 0x1FF;
	uint64_t index_pt = (virtual_address >> 12) & 0x1FF;
	paging_page_entry *pdpt = NULL;
	paging_page_entry *pd = NULL;
	paging_page_entry *pt = NULL;

	debugf( "Examining page for address 0x%016llx:\n", virtual_address );
	if( limine_pml4[index_pml4].present == 0 ) {
		debugf( "PML4 index no present.\n" );
	} else {
		debugf( "PML4 @ index %d:\n", index_pml4 );
		paging_dump_page( &limine_pml4[index_pml4] );
		pdpt = limine_pml4[index_pml4].address << 12;
		
		if( pdpt[index_pdpt].present == 0 ) {
			debugf( "PDPT index not present.\n" );
		} else {
			debugf( "PDPT @ index %d:\n", index_pdpt );
			paging_dump_page( &pdpt[index_pdpt] );
			pd = pdpt[index_pdpt].address << 12;

			if( pd[index_pd].present == 0 ) {
				debugf( "PD index not present.\n" );
			} else {
				debugf( "PD @ index %d:\n", index_pd );
				paging_dump_page( &pd[index_pd] );
				pt = pd[index_pd].address << 12;

				if( pt[index_pt].present == 0 ) {
					debugf( "PT index %d not present.\n", index_pt );
				} else {
					debugf( "PT @ index %d:\n", index_pt );
					paging_dump_page( &pt[index_pt] );
				}
			}
		}
	} */
}

uint64_t paging_virtual_to_physical( uint64_t virtual_address ) {
	uint64_t index_pml4 = (virtual_address >> 39) & 0x1FF;
	uint64_t index_pdpt = (virtual_address >> 30) & 0x1FF;
	uint64_t index_pd = (virtual_address >> 21) & 0x1FF;
	uint64_t index_pt = (virtual_address >> 12) & 0x1FF;
	uint64_t *pml4 = get_cr3() + ADDR_IDENTITY_MAP;
	uint64_t *pdpt = NULL;
	uint64_t *pd = NULL;
	uint64_t *pt = NULL;
	uint64_t physical_address = 0;

	//debugf( "Examining page for address 0x%016llx:\n", virtual_address );
	if( !(pml4[index_pml4] & PAGE_FLAG_PRESENT) ) {
		debugf( "PML4 index no present.\n" );
	} else {

		//paging_dump_page( &limine_pml4[index_pml4] );
		pdpt = (pml4[index_pml4] & PAGE_ADDR_MASK) + ADDR_IDENTITY_MAP;
		
		if( !(pdpt[index_pdpt] & PAGE_FLAG_PRESENT) ) {
			debugf( "PDPT index not present.\n" );
		} else {
			//paging_dump_page( &pdpt[index_pdpt] );
			pd = (pdpt[index_pdpt] & PAGE_ADDR_MASK) + ADDR_IDENTITY_MAP;

			if( !(pd[index_pd] & PAGE_FLAG_PRESENT) ) {
				debugf( "PD index not present.\n" );
			} else {
				//paging_dump_page( &pd[index_pd] );
				pt = (pd[index_pd] & PAGE_ADDR_MASK) + ADDR_IDENTITY_MAP;

				if( !(pt[index_pt] & PAGE_FLAG_PRESENT) ) {
					debugf( "PT index not present.\n" );
				} else {
					//paging_dump_page( &pt[index_pt] );
					physical_address = pt[index_pt] & PAGE_ADDR_MASK;
				}
			}
		}
	}

	return physical_address;
}

paging_page_entry *paging_get_page_for_virtual_address( uint64_t virtual_address ) {
	/* uint64_t index_pml4 = (virtual_address >> 39) & 0x1FF;
	uint64_t index_pdpt = (virtual_address >> 30) & 0x1FF;
	uint64_t index_pd = (virtual_address >> 21) & 0x1FF;
	uint64_t index_pt = (virtual_address >> 12) & 0x1FF;
	paging_page_entry *pdpt = NULL;
	paging_page_entry *pd = NULL;
	paging_page_entry *pt = NULL;
	paging_page_entry *ret_val = NULL;

	//debugf( "Examining page for address 0x%016llx:\n", virtual_address );
	if( limine_pml4[index_pml4].present == 0 ) {
		debugf( "PML4 index no present.\n" );
	} else {

		pdpt = limine_pml4[index_pml4].address << 12;
		
		if( pdpt[index_pdpt].present == 0 ) {
			debugf( "PDPT index not present.\n" );
		} else {
			//paging_dump_page( &pdpt[index_pdpt] );
			pd = pdpt[index_pdpt].address << 12;

			if( pd[index_pd].present == 0 ) {
				debugf( "PD index not present.\n" );
			} else {
				//paging_dump_page( &pd[index_pd] );
				if( pd[index_pd].page_size == 1 ) {
					//paging_examine_page_for_address( &pd[index_pd] );
					return &pd[index_pd];
				}

				pt = pd[index_pd].address << 12;

				if( pt[index_pt].present == 0 ) {
					debugf( "PT index not present.\n" );
				} else {
					ret_val = &pt[index_pt];
				}
			}
		}
	}

	return ret_val; */
}

