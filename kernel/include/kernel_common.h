#ifndef VIOS_KERNEL_COMMON_INCLUDED
#define VIOS_KERNEL_COMMON_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <printf.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <bootstrap.h>
#include <syscall.h>
#include <debug.h>
#include <framebuffer.h>

// From helper_asm.S
extern uint64_t get_cr0( void );
extern uint64_t get_cr2( void );
extern uint64_t get_cr3( void );
extern uint64_t get_cr4( void );
extern void asm_refresh_cr3( void );

typedef struct {
    uint64_t kernel_physical_base;
    uint64_t kernel_virtual_base;
    uint64_t kernel_allocate_memory_start;
    uint64_t kernel_allocate_memory_size;
    uint64_t usable_memory_start;
    uint64_t usable_memory_size;
    uint64_t kernel_start;
    uint64_t kernel_end;
    uint64_t kernel_file_address;
    uint64_t kernel_file_size;
    uint64_t rsdp_table_address;

    framebuffer_information framebuffer_info;

    bool in_paging_sanity_test;
    bool in_page_fault_test;
} kinfo;

#define htonl(l)  ( (((l) & 0xFF) << 24) | (((l) & 0xFF00) << 8) | (((l) & 0xFF0000) >> 8) | (((l) & 0xFF000000) >> 24))
#define htons(s)  ( (((s) & 0xFF) << 8) | (((s) & 0xFF00) >> 8) )
#define ntohl(l)  htonl((l))
#define ntohs(s)  htons((s))


#ifdef __cplusplus
}
#endif
#endif