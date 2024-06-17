#ifndef VIOS_MMIO_INCLUDED
#define VIOS_MMIO_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

class MMIO {
    public:
        uint64_t *addr;

        uint32_t read32( uint64_t *address );
        void write32( uint64_t *address, uint32_t value );

        uint32_t read_command( uint32_t reg );
        void write_command( uint32_t reg, uint32_t value );

        void configure_mmu_page( void );

        MMIO( uint64_t *address );
};

#ifdef __cplusplus
}
#endif
#endif