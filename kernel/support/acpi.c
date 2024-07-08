#include <kernel_common.h>
#include <acpi.h>

extern kinfo kernel_info;

#undef DEBUG_ACPI_INIT
void acpi_initalize( void ) {
	apic_xsdp *system_description_pointer = (apic_xsdp *)kernel_info.rsdp_table_address;

	#ifdef DEBUG_ACPI_INIT
	debugf( "sig: \"%c%c%c%c%c%c%c%c\"\n", system_description_pointer->signature[0], system_description_pointer->signature[1], system_description_pointer->signature[2], system_description_pointer->signature[3], system_description_pointer->signature[4], system_description_pointer->signature[5], system_description_pointer->signature[6], system_description_pointer->signature[7] );
	debugf( "revision: %d\n", system_description_pointer->revision );
	debugf( "rsdt_address: %016llx\n", system_description_pointer->rsdt_address );
	#endif
	
	if( system_description_pointer->revision != 0 ) {
		debugf( "ACPI revision is %d, we only support revision 0.\n", system_description_pointer->revision );
		return;
	}

	apic_rsdt *rsdt = (apic_rsdt *)system_description_pointer->rsdt_address;

	#ifdef DEBUG_ACPI_INIT
	debugf( "sdt signature: %c%c%c%c\n", rsdt->header.signature[0], rsdt->header.signature[1], rsdt->header.signature[2], rsdt->header.signature[3] );
	debugf( "sdt length: %d\n", rsdt->header.length );
	#endif

	apic_madt *madt = (apic_madt *)acpi_find_sdt_header( rsdt, "APIC" );

	if( madt == NULL ) {
		debugf( "MADT not found.\n" );
		return;
	}

	#ifdef DEBUG_ACPI_INIT
	debugf( "local apic address: %X\n", madt->local_apic_address );
	debugf( "length: %d\n", madt->header.length );
	debugf( "flags: %X\n", madt->flags );

	kdebug_peek_at( (uint64_t)madt );
	#endif

	for( int i = 0; i < madt->header.length - sizeof(apic_madt); i++ ) {
		int entry_type = madt->b[i];
		int entry_length = madt->b[i + 1];

		#ifdef DEBUG_ACPI_INIT
		debugf( "entry_type: %d\n", entry_type );
		debugf( "entry_length: %d\n", entry_length );
		#endif

		switch( entry_type ) {
			case APIC_ENTRY_TYPE_PROC_LOCAL_APIC: //processor local apic
				break;
			case APIC_ENTRY_TYPE_IO_APIC:
				break;
			case APIC_ENTRY_TYPE_IO_APIC_ISO:
				break;
			case APIC_ENTRY_TYPE_IO_APIC_NMI_SOURCE:
				break;
			case APIC_ENTRY_TYPE_LOCAL_APIC_NMI:
				break;
			case APIC_ENTRY_TYPE_LOCAL_APIC_ADDR_OVERRIDE:
				break;
			case 6:
			case 7:
			case 8:
				debugf( "unknown entry type found: %d\n", entry_type );
			case APIC_ENTRY_TYPE_PROC_LOCAL_APIC_X2:
				break;
			default:
				debugf( "unknown entry type found: %d\n", entry_type );
		}

		if( entry_type == APIC_ENTRY_TYPE_LOCAL_APIC_ADDR_OVERRIDE ) {
			#ifdef DEBUG_ACPI_INIT
			debugf( "got local apic addr override\n" );
			debugf( "entry size: %d\n", entry_length );
			#endif
			uint64_t *addr_override = (uint64_t *)madt->b[i + 10];

			#ifdef DEBUG_ACPI_INIT
			debugf( "addr override: %X\n", addr_override );
			#endif
		}
		
		i = i + entry_length - 1;
	}
}

apic_sdt_header *acpi_find_sdt_header( apic_rsdt *rsdt, char *header_sig ) {
	int num_entries = (rsdt->header.length - sizeof(apic_sdt_header)) / 4;
	apic_sdt_header *ret_val = NULL;

	for( int i = 0; i < num_entries; i++ ) {
		uint32_t entry_addr = rsdt->sdt[i];
		apic_sdt_header *entry = (apic_sdt_header *)entry_addr;
		
		// debugf( "sdt signature: %c%c%c%c\n", entry->signature[0], entry->signature[1], entry->signature[2], entry->signature[3] );

		if( strncmp( entry->signature, header_sig, 4 ) == 0 ) {
			ret_val = entry;
		}
	}

	return ret_val;
}