#ifndef VIOS_ACPI_INCLUDED
#define VIOS_ACPI_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define APIC_ENTRY_TYPE_PROC_LOCAL_APIC 0
#define APIC_ENTRY_TYPE_IO_APIC 1
#define APIC_ENTRY_TYPE_IO_APIC_ISO 2
#define APIC_ENTRY_TYPE_IO_APIC_NMI_SOURCE 3
#define APIC_ENTRY_TYPE_LOCAL_APIC_NMI 4
#define APIC_ENTRY_TYPE_LOCAL_APIC_ADDR_OVERRIDE 5
#define APIC_ENTRY_TYPE_PROC_LOCAL_APIC_X2 9

typedef struct {
	char signature[8];
	uint8_t checksum;
	char OEMID[6];
	uint8_t revision;
	uint32_t rsdt_address;
	uint32_t length;
	uint64_t xsdt_address;
	uint8_t extended_checksum;
	uint8_t reserved[3];
} __attribute__ ((packed)) apic_xsdp;

typedef struct {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char OEMID[6];
	char OEMTableID[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__ ((packed)) apic_sdt_header;

typedef struct {
    apic_sdt_header header;
    uint32_t sdt[];
} apic_rsdt;

typedef struct {
	apic_sdt_header header;
	uint32_t local_apic_address;
	uint32_t flags;
	uint8_t b[];
} apic_madt;

void acpi_initalize( void );
apic_sdt_header *acpi_find_sdt_header( apic_rsdt *rsdt, char *header_sig );

#ifdef __cplusplus
}
#endif
#endif