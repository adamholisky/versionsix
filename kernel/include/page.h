#ifndef VIOS_PAGE_INCLUDED
#define VIOS_PAGE_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif
#define PAGE_SIZE 0x1000

typedef struct {
    uint8_t     reserved : 3; // 0, 1, 2
    uint8_t     write_through : 1; // 3
    uint8_t     cache_disabled : 1; // 4
    uint32_t    ignored : 7; // 5, 6, 7, 8, 9, 10, 11
    uint64_t    address : 40; // 12 - 52 (max physical address)
} __attribute__ ((packed)) paging_cr3;

typedef struct {
	uint8_t		present : 1; // 0
	uint8_t		rw : 1; // 1
	uint8_t		user_supervisor : 1; // 2
	uint8_t		write_through : 1; // 3
	uint8_t		cache_disabled : 1; // 4
	uint8_t		accessed : 1; // 5

	uint8_t	    ignored_1 : 1; // 6
	uint8_t		page_size : 1; // 7
	uint8_t		ignored_2 : 4; // 8, 9, 10, 11
	
	uint64_t	address : 38; // 12 - 50

	uint8_t		reserved_1 : 1; // 51
	uint16_t    available_2 : 11; // 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62
    
	uint8_t	    execute_disable : 1; // 63
} __attribute__ ((packed)) paging_page_entry;

#define GET_PDE_PRESENT( pde ) pde & 0x1 
#define GET_PDE_READ_WRITE( pde ) pde & 0x2 >> 1
#define GET_PDE_USER_SUPERVISOR( pde ) (pde & 0x4) >> 2
#define GET_PDE_WRITE_THROUGH( pde ) (pde & 0x8) >> 3
#define GET_PDE_CACHE_DISABLED( pde ) (pde & 0x10) >> 4
#define GET_PDE_ACCESSED( pde ) (pde & 0x20) >> 5
#define GET_PDE_PAGE_SIZE( pde ) (pde & 0x80) >> 7
#define GET_PDE_EXECUTE_DISABLED( pde ) pde >> 63
#define GET_PDE_ADDRESS( pde ) (pde & 0x000FFFFFFFFF000) >> 12

#define SET_PDE_PRESENT( pde ) pde | (1 << 0)
#define SET_PDE_READ_WRITE( pde ) pde | (1 << 1)
#define SET_PDE_USER_SUPERVISOR( pde ) pde | (1 << 2)
#define SET_PDE_WRITE_THROUGH( pde ) pde | (1 << 3)
#define SET_PDE_CACHE_DISABLED( pde ) pde | (1 << 4)
#define SET_PDE_ACCESSED( pde ) pde | (1 << 5)
#define SET_PDE_PAGE_SIZE( pde ) pde | (1 << 7)
#define SET_PDE_EXECUTE_DISABLED( pde ) pde | (1 << 63)
#define SET_PDE_ADDRESS( pde, addr ) pde | (addr << 12)


void paging_initalize( void );
uint64_t *page_allocate( uint8_t number );
uint64_t *page_allocate_kernel( uint32_t number );
uint64_t *page_allocate_kernel_mmio( uint8_t number );
void paging_dump_page_direct( uint64_t page );
void paging_dump_cr3( paging_cr3 *cr3 );
void paging_dump_page( paging_page_entry *page );
uint64_t *page_map( uint64_t virtual_address, uint64_t physical_address );
void paging_examine_page_for_address( uint64_t virtual_address );
uint64_t paging_virtual_to_physical( uint64_t virtual_address );
paging_page_entry *paging_get_page_for_virtual_address( uint64_t virtual_address );

#ifdef __cplusplus
}
#endif
#endif