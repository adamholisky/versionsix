#if !defined(VIOS_DEVICE_INCLUDED)
#define VIOS_DEVICE_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
    char major_id[7];
    char minor_id[7];

    void (*close)( void );
    void (*open)( void );
    uint8_t (*read)( void );
    void (*write)( void *, size_t );
} device;

typedef struct {
    device *dev;
    void *next;
} device_list;

void device_initalize( void );
void devices_populate_fs( void );
bool devices_setup( void );
void device_register( device *d );
device *device_get_major_minor_device( char *major, char *minor );

#ifdef __cplusplus
}
#endif
#endif