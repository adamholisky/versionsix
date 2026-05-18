#include <kernel_common.h>
#include <syscall.h>
#include <process.h>
#include <vfs.h>
#include <page.h>
#include <kmemory.h>

extern kernel_proc_data global_proc_data;

#define current_proc global_proc_data.current_process

/**
 * @brief Sets the end of the heap to the given address, assuming it makes sense to the kernel
 * 
 * @param addr Address to set the end of the heap to. Must be larger than current end, must not exceed program limits.
 * @return int 0 on success, -1/error otherwise
 */
int brk( void *addr ) {
	syscall_args args = {
		.arg_1 = addr
	};

	return syscall( SYSCALL_BRK, 1, &args );
}

int brk_syscall_handler( registers **context, void *addr ) {
	if( addr <= current_proc->heap_top ) {
		klog( LOG_ERROR, "Trying to deallocate memory. pid=%d  addr=0x%X", current_proc->pid, addr );
		return -1;
	}

	// If we already have enough pages allocated, then increase the top to the new address, return success
	if( addr <= current_proc->heap_top_allocated ) {
		current_proc->heap_top = addr;
		return 0;
	}

	// We need more pages...
	uint64_t heap_add = (uint64_t)addr - (uint64_t)current_proc->heap_top;
	uint16_t num_pages = (heap_add % PAGE_SIZE ) + 1;

	current_proc->heap_pages = krealloc( current_proc->heap_pages, sizeof(process_exec_section) * (current_proc->heap_page_count + num_pages) );

	void *old_heap_top = current_proc->heap_top;

	for( int i = 0; i < num_pages; i++ ) {
		current_proc->heap_pages[i].kern_virt = page_allocate(1);
		current_proc->heap_pages[i].phys = paging_virtual_to_physical( current_proc->heap_pages[i].kern_virt );
		current_proc->heap_pages[i].virt = current_proc->heap_top_allocated;

		current_proc->heap_top_allocated = current_proc->heap_top_allocated + PAGE_SIZE;
	}

	// Now that we have the pages, set heap top and return success
	current_proc->heap_top = addr;

	klog( LOG_INFO, "Heap expanded. added=%X  pages=%d  old_top=%X  new_top=%X", heap_add, num_pages, old_heap_top, current_proc->heap_top );

	return 0;
}

/**
 * @brief Increments the process's heap by the given bytes.
 * 
 * @param inc_bytes Number of bytes to increase heap size by.
 * @return void* Address of the start of the newly allocated memory. If inc_bytes is 0, then returns the address of the top of the heap.
 */
void* sbrk( int inc_bytes ) {
	syscall_args args = {
		.arg_1 = inc_bytes
	};

	return syscall( SYSCALL_SBRK, 1, &args );
}

void* sbrk_syscall_handler( registers **context, int inc_bytes ) {
	int brk_res = brk_syscall_handler( context, current_proc->heap_top + inc_bytes );
	
	if( brk_res != 0 ) {
		return NULL;
	} else {
		return current_proc->heap_top;
	}
}