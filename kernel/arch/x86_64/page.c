#include "kernel_common.h"
#include "page.h"
#include <string.h>

paging_page_entry *limine_pml4;
paging_page_entry *limine_pdpt;
paging_page_entry *limine_pd;
paging_page_entry *limine_pt;
paging_page_entry *limine_next_open_pt;
uint64_t kernel_virtual_memory_next;
uint64_t kernel_physical_memory_next;
uint32_t limine_pt_next_free;
uint32_t limine_pd_count;
uint64_t physical_base;
uint64_t virtual_base;
uint64_t kernel_heap_virtual_memory_next;
uint64_t kernel_heap_physical_memory_next;

paging_page_entry kernel_pts[2][512][512] __attribute__ ((aligned (4096)));
paging_page_entry kernel_pds[2][512] __attribute__ ((aligned (4096)));
paging_page_entry kernel_pdpt[2] __attribute__ ((aligned (4096)));

extern kinfo kernel_info;

// Intentionally turning off warnings given we need to smash data together
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#pragma GCC diagnostic ignored "-Wint-conversion"

#undef DEBUG_PAGING_INITALIZE
void paging_initalize( void ) {
    uint64_t cr3 = get_cr3();
    paging_cr3 *paging_cr3_data = (paging_cr3 *)&cr3;

    
    limine_pml4 = (paging_page_entry *)(paging_cr3_data->address << 12);
    limine_pdpt = (paging_page_entry *)(limine_pml4[0x1FF].address << 12);
    //limine_pd = (paging_page_entry *)(limine_pdpt[0x1FE].address << 12);
    //limine_pt = (paging_page_entry *)(limine_pd[0].address << 12);
    limine_pt_next_free = 0;
    virtual_base = kernel_info.kernel_virtual_base;
    physical_base = kernel_info.kernel_physical_base;
    kernel_virtual_memory_next = kernel_info.kernel_end;
    kernel_physical_memory_next = kernel_info.kernel_allocate_memory_start;
    kernel_heap_virtual_memory_next = 0xEEEEEEEE00000000;
    kernel_heap_physical_memory_next = kernel_info.usable_memory_start;

    // Copy over each page entry that's setup by the bootloader into our tables.
    // 1. Loop through each PDPT entry
    for( int i = 0x1FE; i < 512; i++ ) {

        // 2. If the PDPT entry is present, then go into the PD entry
        if( limine_pdpt[i].present == 1 ) {
            limine_pd = (paging_page_entry *)(limine_pdpt[i].address << 12);

            //debugf( "addr: %016llx\n", ((uint64_t)((uint64_t)(kernel_pds + (i - 510)) - virtual_base + physical_base) >> 12));
            kernel_pdpt[i - 510].present = 1;
            kernel_pdpt[i - 510].rw = 1;
            kernel_pdpt[i - 510].address = (uint64_t)((uint64_t)(kernel_pds + (i - 510)) - virtual_base + physical_base) >> 12;
        
            // 3. Loop through each PD entry
            for( int j = 0; j < 512; j++ ) {

                //debugf( "j = %d\n", j );
                // 4. If the PD entry is present, then go into the PT
                if( limine_pd[j].present == 1 ) {
                    limine_pt = (paging_page_entry *)(limine_pd[j].address << 12);

                    kernel_pds[i - 510][j].address = (uint64_t)(((uint64_t)(&kernel_pts[i - 0x1FE][j][0]) - virtual_base + physical_base) >> 12);
                    kernel_pds[i - 510][j].present = 1;
                    kernel_pds[i - 510][j].rw = 1;

                    // 5. Loop throuhg each PT entry, copy it into the kernel tables
                    for( int k = 0; k < 512; k++ ) {
                        uint64_t *bootloader_pt_entry = (uint64_t *)(limine_pt + k);

                        paging_page_entry *pt_entry = &kernel_pts[i - 0x1FE][j][k];
                        uint64_t *kernel_pt_entry = (uint64_t *)(pt_entry);

                        *kernel_pt_entry = *bootloader_pt_entry;

                        if( kernel_pts[i - 0x1FE][j][k].present != 0 ) {
                            //debugf( "k pt entry: kernel_pt[0x%03X][%d][%d] = 0x%016llX\n", i, j, k, kernel_pts[i - 0x1FE][j][k] );
                        }
                    }
                }
            }
        }
    }

    #ifdef DEBUG_PAGING_INITALIZE
    debugf( "0x1fe addr: %016llx\n", limine_pdpt[0x1fe].address << 12 );
    debugf( "addr: %016llx\n", ((uint64_t)((uint64_t)(&kernel_pds[0][0]) - virtual_base + physical_base) >> 12));
    #endif
    
    limine_pdpt[0x1FE].address = ((uint64_t)((uint64_t)(&kernel_pds[0][0]) - virtual_base + physical_base) >> 12);
    
    #ifdef DEBUG_PAGING_INITALIZE
    debugf( "0x1fe addr: %016llx\n", limine_pdpt[0x1fe].address << 12 );
    #endif

    #ifdef DEBUG_PAGING_INITALIZE
    debugf( "0x1ff addr: %016llx\n", limine_pdpt[0x1ff].address << 12 );
    debugf( "addr: %016llx\n", ((uint64_t)((uint64_t)(&kernel_pds[1][0]) - virtual_base + physical_base) >> 12));
    #endif
    
    limine_pdpt[0x1Ff].address = ((uint64_t)((uint64_t)(&kernel_pds[1][0]) - virtual_base + physical_base) >> 12);
    
    #ifdef DEBUG_PAGING_INITALIZE
    debugf( "0x1ff addr: %016llx\n", limine_pdpt[0x1ff].address << 12 );
    #endif

    asm_refresh_cr3();

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

#undef DEBUG_PAGE_ALLOCATE
uint64_t *page_allocate( uint8_t number ) {
    uint64_t *return_val = NULL;

    #ifdef DEBUG_PAGE_ALLOCATE
    log_entry_enter();
    #endif

    if( number < 1 ) {
        return NULL;
    }

    for( int i = 0; i < number; i++ ) {
        if( i == 0 ) {
            return_val = page_map( kernel_heap_virtual_memory_next, kernel_heap_physical_memory_next );   
        } else {
            page_map( kernel_heap_virtual_memory_next, kernel_heap_physical_memory_next );   
        }
        
        kernel_heap_virtual_memory_next = kernel_heap_virtual_memory_next + 0x1000;
        kernel_heap_physical_memory_next = kernel_heap_physical_memory_next + 0x1000;
    }

    #ifdef DEBUG_PAGE_ALLOCATE
    log_entry_exit();
    #endif

    //debugf( "Page allocated: 0x%016llX\n", return_val );
    return return_val;
}

#undef DEBUG_PAGE_ALLOCATE_KERNEL
uint64_t *page_allocate_kernel( uint32_t number ) {
    uint64_t *return_val = NULL;

    #ifdef DEBUG_PAGE_ALLOCATE_KERNEL
    log_entry_enter();
    #endif

    if( number < 1 ) {
        return NULL;
    }

    int i;
    for( i = 0; i < number; i++ ) {
        if( i == 0 ) {
            return_val = page_map( kernel_virtual_memory_next, kernel_physical_memory_next );
        } else {
            page_map( kernel_virtual_memory_next, kernel_physical_memory_next );
        }
        
        kernel_virtual_memory_next = kernel_virtual_memory_next + 0x1000;
        kernel_physical_memory_next = kernel_physical_memory_next + 0x1000;
    }

    #ifdef DEBUG_PAGE_ALLOCATE_KERNEL
    log_entry_exit();
    #endif

    //debugf( "Page allocated (kernel): addr=0x%016llX  number=%d  i=%d\n", return_val, number, i );
    return return_val;
}

uint64_t *page_allocate_kernel_mmio( uint8_t number ) {
    uint64_t *return_val = NULL;
    uint64_t *next_memory = NULL;

    return_val = page_allocate_kernel( number );

    next_memory = return_val;

    if( return_val == NULL ) {
        df( "return_val is null\n" );
    }

    for( int i = 0; i < number; i++ ) {
        paging_page_entry *page = paging_get_page_for_virtual_address( (uint64_t)next_memory );
        
        page->cache_disabled = 1;
        page->write_through = 1;

        next_memory = (uint64_t *)((uint64_t)next_memory + PAGE_SIZE);
    }

    asm_refresh_cr3();

    return return_val;
}

#undef DEBUG_PAGE_MAP
uint64_t *page_map( uint64_t virtual_address, uint64_t physical_address ) {
    #ifdef DEBUG_PAGE_MAP
        debugf( "page_map( virtual_address = 0x%016llX, physical_address = 0x%016llx )\n", virtual_address, physical_address );
    #endif

    // Find indexes
    uint64_t index_pml4 = (virtual_address >> 39) & 0x1FF;
    uint64_t index_pdpt = (virtual_address >> 30) & 0x1FF;
    uint64_t index_pd = (virtual_address >> 21) & 0x1FF;
    uint64_t index_pt = (virtual_address >> 12) & 0x1FF;
    bool setup_pml4 = false;
    bool setup_pdpt = false;
    bool setup_pd = false;
    bool setup_pt = false;
    paging_page_entry *pdpt = NULL;
    paging_page_entry *pd = NULL;
    paging_page_entry *pt = NULL;

    /*
    1. Figure out what we need to setup for each level
    2. Setup the directories/tables for each level
    3. Go back and fill in addresses for each level
    4. Return virtual address, otherwise NULL (failure)
    */

    if( limine_pml4[index_pml4].present == 0 ) {
        setup_pml4 = true;
        setup_pdpt = true;
        setup_pd = true;
        setup_pt = true;
    } else {
        pdpt = limine_pml4[index_pml4].address << 12;
        
        if( pdpt[index_pdpt].present == 0 ) {
            setup_pdpt = true;
            setup_pd = true;
            setup_pt = true;
        } else {
            pd = pdpt[index_pdpt].address << 12;

            if( pd[index_pd].present == 0 ) {
                setup_pd = true;
                setup_pt = true;
            } else {
                pt = pd[index_pd].address << 12;

                if( pt[index_pt].present == 0 ) {
                    setup_pt = true;
                } else {
                   // Already assigned, do something?
                }
            }
        }
    }

    #ifdef DEBUG_PAGE_MAP
    debugf( "Pre-setup pdpt: 0x%016llx\n", pdpt );
    debugf( "Pre-setup pd:   0x%016llx\n", pd );
    debugf( "Pre-setup pt:   0x%016llx\n", pt );
    debugf( "setup_pml4: %d @ index 0x%X\n", setup_pml4, index_pml4 );
    debugf( "setup_pdpt: %d @ index 0x%X\n", setup_pdpt, index_pdpt );
    debugf( "setup_pd: %d @ index 0x%X\n", setup_pd, index_pd );
    debugf( "setup_pt: %d @ index 0x%X\n", setup_pt, index_pt );
    #endif

    // If we're allocating a kernel address, use the allocated PTs, otherwise assign
    if( virtual_address > virtual_base ) {
        if( setup_pdpt ) {
            pdpt = (paging_page_entry *)&limine_pdpt[index_pdpt];

            #ifdef DEBUG_PAGE_MAP
            debugf( "pdpt: %llx\n", pdpt );
            #endif
        }

        if( setup_pd ) {
            pd = (paging_page_entry *)&kernel_pds[index_pdpt - 510][index_pd];

            #ifdef DEBUG_PAGE_MAP
            debugf( "pd: %llx\n", pd );
            #endif
        }

        if( setup_pt ) {
            pt = (paging_page_entry *)&kernel_pts[index_pdpt - 510][index_pd][0];

            #ifdef DEBUG_PAGE_MAP
            debugf( "pt: %llx\n", pt );
            #endif
        }
    } else {
        if( setup_pdpt ) {
            pdpt = (paging_page_entry *)page_allocate_kernel(1);
            memset( pdpt, 0, 4096 );

            #ifdef DEBUG_PAGE_MAP
            debugf( "allocated pdpt: %llx\n", pdpt );
            #endif
        }

        if( setup_pd ) {
            pd = (paging_page_entry *)page_allocate_kernel(1);
            memset( pd, 0, 4096 );

            #ifdef DEBUG_PAGE_MAP
            debugf( "allocated pd: %llx\n", pd );
            #endif
        }

        if( setup_pt ) {
            pt = (paging_page_entry *)page_allocate_kernel(1);
            memset( pt, 0, 4096 );
            
            #ifdef DEBUG_PAGE_MAP
            debugf( "allocated pt: %llx\n", pt );
            #endif
        }
    }

    pt[index_pt].address = physical_address >> 12;
    pt[index_pt].rw = 1;
    pt[index_pt].present = 1;
    pt[index_pt].cache_disabled = 1;
    pt[index_pt].write_through = 1;

    uint64_t physical_base_modifier = 0;
    uint64_t virtual_base_modifier = 0;
    if( virtual_address > virtual_base ) {
        physical_base_modifier = physical_base;
        virtual_base_modifier = virtual_base;
    } else {
        physical_base_modifier = kernel_info.kernel_allocate_memory_start;
        virtual_base_modifier = kernel_info.kernel_end;
    }

    uint64_t pt_physical_addr = (uint64_t)pt - virtual_base_modifier + physical_base_modifier;

    if( setup_pd ) {
        pd[index_pd].address = pt_physical_addr >> 12;
        pd[index_pd].rw = 1;
        pd[index_pd].present = 1;

        //debugf( "setup_pd triggered\n" );
    }

    uint64_t pd_physical_addr = (uint64_t)pd - virtual_base_modifier + physical_base_modifier;

    if( setup_pdpt ) {
        pdpt[index_pdpt].address = pd_physical_addr >> 12;
        pdpt[index_pdpt].rw = 1;
        pdpt[index_pdpt].present = 1;
        //debugf( "setup_pdpt triggered\n" );
    } else if( setup_pd ) {
        pdpt[index_pdpt].address = pd_physical_addr >> 12;
    }

    uint64_t pdpt_physical_addr = (uint64_t)pdpt - virtual_base_modifier + physical_base_modifier;

    if( setup_pml4 ) { 
        limine_pml4[index_pml4].address = pdpt_physical_addr >> 12;
        limine_pml4[index_pml4].rw = 1;
        limine_pml4[index_pml4].present = 1;
    }

    asm_refresh_cr3();

    #ifdef DEBUG_PAGE_MAP

    paging_dump_page( &pt[index_pt] );
    paging_dump_page( &pd[index_pd] );
    paging_dump_page( &pdpt[index_pdpt] );
    paging_dump_page( &limine_pml4[index_pml4] );

    debugf( "pt_physical_addr: %llx\n", pt_physical_addr );
    debugf( "pd_physical_addr: %llx\n", pd_physical_addr );
    debugf( "pdpt_physical_addr: %llx\n", pdpt_physical_addr );
    debugf( "virtual_addr: %llx\n", virtual_address );
    #endif
    return (uint64_t *)virtual_address;
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
    uint64_t index_pml4 = (virtual_address >> 39) & 0x1FF;
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

        paging_dump_page( &limine_pml4[index_pml4] );
        pdpt = limine_pml4[index_pml4].address << 12;
        
        if( pdpt[index_pdpt].present == 0 ) {
            debugf( "PDPT index not present.\n" );
        } else {
            paging_dump_page( &pdpt[index_pdpt] );
            pd = pdpt[index_pdpt].address << 12;

            if( pd[index_pd].present == 0 ) {
                debugf( "PD index not present.\n" );
            } else {
                paging_dump_page( &pd[index_pd] );
                pt = pd[index_pd].address << 12;

                if( pt[index_pt].present == 0 ) {
                    debugf( "PT index not present.\n" );
                } else {
                    paging_dump_page( &pt[index_pt] );
                }
            }
        }
    }
}

uint64_t paging_virtual_to_physical( uint64_t virtual_address ) {
    uint64_t index_pml4 = (virtual_address >> 39) & 0x1FF;
    uint64_t index_pdpt = (virtual_address >> 30) & 0x1FF;
    uint64_t index_pd = (virtual_address >> 21) & 0x1FF;
    uint64_t index_pt = (virtual_address >> 12) & 0x1FF;
    paging_page_entry *pdpt = NULL;
    paging_page_entry *pd = NULL;
    paging_page_entry *pt = NULL;
    uint64_t physical_address = 0;

    //debugf( "Examining page for address 0x%016llx:\n", virtual_address );
    if( limine_pml4[index_pml4].present == 0 ) {
        debugf( "PML4 index no present.\n" );
    } else {

        //paging_dump_page( &limine_pml4[index_pml4] );
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
                pt = pd[index_pd].address << 12;

                if( pt[index_pt].present == 0 ) {
                    debugf( "PT index not present.\n" );
                } else {
                    //paging_dump_page( &pt[index_pt] );
                    physical_address = pt[index_pt].address << 12;
                }
            }
        }
    }

    return physical_address;
}

paging_page_entry *paging_get_page_for_virtual_address( uint64_t virtual_address ) {
    uint64_t index_pml4 = (virtual_address >> 39) & 0x1FF;
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

    return ret_val;
}