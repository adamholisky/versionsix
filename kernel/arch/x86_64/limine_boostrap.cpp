#include <kernel_common.h>
#include <limine_bootstrap.h>
#include <limine.h>

#define LIMINE_KERNEL_ADDRESS_REQUEST { LIMINE_COMMON_MAGIC, 0x71ba76863cc55f63, 0xb2644a48c516a487 }
static volatile struct limine_kernel_address_request kaddr_request = {
	.id = LIMINE_KERNEL_ADDRESS_REQUEST,
	.revision = 0
};

#define LIMINE_KERNEL_FILE_REQUEST { LIMINE_COMMON_MAGIC, 0xad97e90e83f1ed67, 0x31eb5d1c5ff23b69 }
static volatile struct limine_kernel_file_request kfile_request = {
	.id = LIMINE_KERNEL_FILE_REQUEST,
	.revision = 0
};

#define LIMINE_HHDM_REQUEST { LIMINE_COMMON_MAGIC, 0x48dcf1cb8ad2b852, 0x63984e959a98244b }
static volatile struct limine_hhdm_request hhdm_request = {
	.id = LIMINE_HHDM_REQUEST,
	.revision = 0
};

#define LIMINE_MEMMAP_REQUEST { LIMINE_COMMON_MAGIC, 0x67cf3d9d378a806f, 0xe304acdfc50c3c62 }
static volatile struct limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST,
	.revision = 0
};

#define LIMINE_FRAMEBUFFER_REQUEST { LIMINE_COMMON_MAGIC, 0x9d5827dcd881dd75, 0xa3148604f6fab11b }
static volatile struct limine_framebuffer_request fb_request {
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0
};

extern kinfo kernel_info;
extern uint64_t _kernel_start;
extern uint64_t _kernel_end;
extern uint64_t kernel_main;

void load_limine_info( void ) {
    uint64_t usable_memory_start = 0;
	uint64_t usable_memory_size = 0;

    debugf( "kernel_main:          0x%p\n", kernel_main );
	debugf( "Physical kernel base: 0x%016llx\n", kaddr_request.response->physical_base );
	debugf( "Virtaul kernel base:  0x%016llx\n", kaddr_request.response->virtual_base );
	debugf( "Virtual kstart:       0x%016llx\n", &_kernel_start );
	debugf( "Virtual kend:         0x%016llx\n", &_kernel_end );
	debugf( "cr0:                  0x%llx\n", get_cr0() );
	debugf( "cr3:                  0x%llx\n", get_cr3() );
	debugf( "cr4:                  0x%llx\n", get_cr4() );

	// Get the memory map and iterate over each entrry, saving the largest contiguous block as the physical base for our allocateable memory
	debugf( "Memory Map:\n" );
	debugf_raw( "          Base                Length              Type\n" );


	debugf( "usable_memory_start: 0x%016llx  size: 0x%016llx\n", usable_memory_start, usable_memory_size );
	debugf( "kernel_allocate_memory_start: 0x%016llx  size: 0x%016llx\n", kernel_info.kernel_allocate_memory_start, kernel_info.kernel_allocate_memory_size );
	debugf( "kfile->path: %s\n", kfile_request.response->kernel_file->path );
	debugf( "kfile->address: 0x%016llX\n", kfile_request.response->kernel_file->address );
	debugf( "kfile->size: 0x%X\n", kfile_request.response->kernel_file->size );
	debugf( "hhdm offset: 0x%016llX\n", hhdm_request.response->offset );
	
    for( int i = 0; i < memmap_request.response->entry_count; i++ ) {
		debugf_raw( "    0x%02X  0x%016llX  0x%016llX  0x%X\n", i, memmap_request.response->entries[i]->base,  memmap_request.response->entries[i]->length,  memmap_request.response->entries[i]->type );

		if( memmap_request.response->entries[i]->type == 0 ) {
			if( memmap_request.response->entries[i]->length > usable_memory_size ) {
				// This is intentional -- kernel pages will use the second largest free space
				if( usable_memory_size != 0 ) {
					kernel_info.kernel_allocate_memory_start = usable_memory_start;
					kernel_info.kernel_allocate_memory_size = usable_memory_size;
				}
				usable_memory_start = memmap_request.response->entries[i]->base;
				usable_memory_size = memmap_request.response->entries[i]->length;
			}
		}
	}

	kernel_info.kernel_start = (uint64_t)&_kernel_start;
	kernel_info.kernel_end = (uint64_t)&_kernel_end;
	kernel_info.kernel_physical_base = kaddr_request.response->physical_base;
	kernel_info.kernel_virtual_base = kaddr_request.response->virtual_base;
	kernel_info.usable_memory_start = usable_memory_start;
	kernel_info.usable_memory_size = usable_memory_size;
	kernel_info.kernel_file_address = (uint64_t)kfile_request.response->kernel_file->address;
	kernel_info.kernel_file_size = kfile_request.response->kernel_file->size;
	kernel_info.framebuffer_info.address = (uint8_t*)fb_request.response->framebuffers[0]->address;
	kernel_info.framebuffer_info.bpp = fb_request.response->framebuffers[0]->bpp;
	kernel_info.framebuffer_info.height = fb_request.response->framebuffers[0]->height;
	kernel_info.framebuffer_info.width = fb_request.response->framebuffers[0]->width;
	kernel_info.framebuffer_info.pitch = fb_request.response->framebuffers[0]->pitch;
	kernel_info.framebuffer_info.pixel_width = kernel_info.framebuffer_info.pitch / kernel_info.framebuffer_info.width;
}