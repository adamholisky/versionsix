#include <kernel_common.h>
#include <limine.h>
#include <serial.h>
#include <file.h>
#include <interrupt.h>
#include <timer.h>
#include <page.h>
#include <kmemory.h>
#include <kshell.h>
#include <ksymbols.h>
#include <pci.h>
#include <e1000.h>
#include <task.h>
#include <elf.h>
#include <gui/console.h>

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

/* __attribute__((used, section(".requests_start_marker"))) static volatile LIMINE_REQUESTS_START_MARKER; 
__attribute__((used, section(".requests_end_marker"))) static volatile LIMINE_REQUESTS_END_MARKER; */

extern uint64_t _kernel_start;
extern uint64_t _kernel_end;

kinfo kernel_info;

extern "C" void kernel_main( void ) {
	serial_initalize();

	printf( "Versions OS VI\n" );
	debugf( "Versions OS VI Debug Out\n" );
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
	uint64_t usable_memory_start = 0;
	uint64_t usable_memory_size = 0;
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

	debugf( "usable_memory_start: 0x%016llx  size: 0x%016llx\n", usable_memory_start, usable_memory_size );
	debugf( "kernel_allocate_memory_start: 0x%016llx  size: 0x%016llx\n", kernel_info.kernel_allocate_memory_start, kernel_info.kernel_allocate_memory_size );
	debugf( "kfile->path: %s\n", kfile_request.response->kernel_file->path );
	debugf( "kfile->address: 0x%016llX\n", kfile_request.response->kernel_file->address );
	debugf( "kfile->size: 0x%X\n", kfile_request.response->kernel_file->size );
	debugf( "hhdm offset: 0x%016llX\n", hhdm_request.response->offset );

	interrupt_initalize();
	timer_initalize();
	paging_initalize();
	memory_initalize();
	kernel_symbols_initalize();
	pci_initalize();
	e1000_initalize();
	framebuffer_initalize();

	//task_initalize();

	//do_divide_by_zero();

	//__asm__ __volatile__ ("int $43");

	Console *main_console = new Console( 0, 0, kernel_info.framebuffer_info.width, kernel_info.framebuffer_info.height );
	main_console->put_string( "Hello, world!\n" );
	main_console->put_string( "This is another" );
	main_console->put_string( " line!" );

	kshell();

	debugf( "Ending happy.\n" );
	printf( "Ending happy.\n" );
	do_immediate_shutdown();
}

typedef struct {
	uint8_t destination[6];
	uint8_t source[6];
	uint16_t type;
} __attribute__((packed)) ethernet_packet;

typedef struct {
    uint16_t htype; // Hardware type
    uint16_t ptype; // Protocol type
    uint8_t  hlen; // Hardware address length (Ethernet = 6)
    uint8_t   plen; // Protocol address length (IPv4 = 4)
    uint16_t opcode; // ARP Operation Code
    uint8_t   src_hardware_addr[6]; // Source hardware address - hlen bytes (see above)
    uint8_t   src_protocol_addr[4]; // Source protocol address - plen bytes (see above). If IPv4 can just be a "u32" type.
    uint8_t   dst_hardware_addr[6]; // Destination hardware address - hlen bytes (see above)
    uint8_t   dst_protocol_addr[4]; // Destination protocol address - plen bytes (see above). If IPv4 can just be a "u32" type.
} __attribute__((packed)) arp_packet;

#define htons(s)  ( (((s) & 0xFF) << 8) | (((s) & 0xFF00) >> 8) )

extern "C" void do_test_send( void ) {
	arp_packet packet;

	packet.htype = 0x1;
	packet.ptype = htons(0x0800);
	packet.hlen = 0x6;
	packet.plen = 0x4;
	packet.opcode = htons(0x001);

	packet.src_hardware_addr[0] = 0x12;
	packet.src_hardware_addr[1] = 0x34;
	packet.src_hardware_addr[2] = 0x56;
	packet.src_hardware_addr[3] = 0x78;
	packet.src_hardware_addr[4] = 0x9A;
	packet.src_hardware_addr[5] = 0xBC;

	packet.src_protocol_addr[0] = 10;
	packet.src_protocol_addr[1] = 0;
	packet.src_protocol_addr[2] = 2;
	packet.src_protocol_addr[3] = 14;

	for( int i = 0; i < 6; i++ ) {
		packet.dst_hardware_addr[i] = 0xFF;
	}

	for( int i = 0; i < 4; i++ ) {
		packet.dst_protocol_addr[i] = 0xFF;
	}

	ethernet_packet epacket;

	epacket.source[0] = 0x12;
	epacket.source[1] = 0x34;
	epacket.source[2] = 0x56;
	epacket.source[3] = 0x78;
	epacket.source[4] = 0x9A;
	epacket.source[5] = 0xBC;

	epacket.type = htons( 0x0806 );

	for( int i = 0; i < 6; i++ ) {
		epacket.destination[i] = 0xFF;
	}

	uint32_t total_size = sizeof( ethernet_packet ) + sizeof( arp_packet );
	uint8_t *to_send = kmalloc( total_size );

	memcpy( to_send, (void *)&epacket, sizeof( ethernet_packet ) );
	memcpy( to_send + sizeof( ethernet_packet ), (void *)&packet, sizeof( arp_packet) );

	kdebug_peek_at( (uint64_t)to_send );

	e1000_send( to_send, total_size );
}