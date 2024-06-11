#include <kernel_common.h>
#include <page.h>
#include <kmemory.h>

extern kinfo kernel_info;

uint64_t memory_top;
uint64_t memory_next_free;

#undef DEBUG_MEMORY_INITALIZE
void memory_initalize( void ) {
    #ifdef DEBUG_MEMORY_INITALIZE
    log_entry_enter();
    #endif

    memory_top = (uint64_t)page_allocate_kernel( 1 ) + 0x1000;
    memory_next_free = memory_top - 0x1000;

    #ifdef DEBUG_MEMORY_INITALIZE
    debugf( "memory_top: %llx\n", memory_top );
    debugf( "memory_next_free: %llx\n", memory_next_free );
    #endif

    uint64_t *mem_test = kmalloc( 256 );
    *mem_test = 0x1010202030304040;
    mem_test = kmalloc( PAGE_SIZE * 2 );
    *mem_test = 0x0000111100002222;
    debugf( "Allocation sanity check: pass\n" );

    #ifdef DEBUG_MEMORY_INITALIZE
    log_entry_exit();
    #endif
}

#undef DEBUG_KMALLOC
void *kmalloc( uint64_t size ) {
    uint64_t *ret_val = (uint64_t *)memory_next_free;

    #ifdef DEBUG_KMALLOC
    log_entry_enter();
    debugf( "memory_next: %llx\n", memory_next_free );
    #endif

    if( memory_next_free + size >= memory_top ) {
        uint16_t num_pages = size/PAGE_SIZE;
        num_pages++;

        #ifdef DEBUG_KMALLOC
        debugf( "malloc allocating num_pages: %x\n", num_pages );
        #endif

        page_allocate_kernel( num_pages );

        memory_top = memory_top + (num_pages * PAGE_SIZE );

        #ifdef DEBUG_KMALLOC
        debugf( "pages allocated: %d\n", num_pages );
        debugf( "memory_top: %llx\n", memory_top );
        #endif
    }

    memory_next_free = memory_next_free + size;

    #ifdef DEBUG_KMALLOC
    debugf( "memory_next_free: %llx\n", memory_next_free );
    debugf( "pointer returned: %llx\n", ret_val );
    log_entry_exit();
    #endif

    return (void *)ret_val;
}

#undef DEBUG_KFREE
void kfree( uint64_t *p ) {

}