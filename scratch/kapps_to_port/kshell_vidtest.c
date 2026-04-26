#include <kernel_common.h>
#include <kshell_app.h>
#include <pci.h>
#include <page.h>

KSHELL_COMMAND( vt, kshell_app_vt_main )

int kshell_app_vt_main( int argc, char *argv[] ) {
	pci_header *vid_pci = pci_get_header_by_device_id(0x1111);

    dpf( "Bar0: 0x%llX\n", vid_pci->bar0);
    dpf( "Bar1: 0x%llX\n", vid_pci->bar1 );

    pci_dump_header(vid_pci);

    void *new_fb = page_allocate_kernel(1024);

    dpf( "new_fb addr: 0x%llX\n", paging_virtual_to_physical(new_fb) );
    pci_write( 0, 2, 0, PCI_BAR_0,paging_virtual_to_physical(new_fb) );

    uint32_t *fb = new_fb;

    for( int i = 0; i < 1024; i++ ) {
        debugf( "i: %d ", i );
        for( int j = 0; j < 768; j++ ) {
            *(fb + (i*1024) + (j)) = 0x00FF0000;
        }
    }

    pci_write( 0, 2, 0, PCI_BAR_0, vid_pci->bar0 );

    //dpf( "Phys fb addr: 0x%llX\n", paging_virtual_to_physical(0xFFFF8000FD000000) );

}