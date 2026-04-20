#if !defined(VIOS_DEVICE_INCLUDED)
#define VIOS_DEVICE_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <vfs.h>

typedef struct {
	char major_id[7];
	char minor_id[7];

	void (*close)( inode_id );
	int (*open)( inode_id );
	int (*read)( inode_id, uint8_t *, uint64_t, uint64_t );
	int (*stat)( inode_id, vfs_stat_data * );
	int (*write)( inode_id, uint8_t *, size_t, size_t );
	int (*interrupt_handler)();
} device;

void device_initalize( void );
void devices_populate_fs( void );
bool devices_setup( void );
void device_register( device *d );
device *device_get_major_minor_device( char *major, char *minor );
void device_list_populate_for_each_callback( avs_node *n );

#ifdef __cplusplus
}
#endif
#endif