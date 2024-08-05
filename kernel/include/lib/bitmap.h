#ifndef VIOS_VIBITMAP_INCLUDED
#define VIOS_VIBITMAP_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include "vit.h"

#define VI_BITMAP_FIELD_TYPE uint64_t
#define VI_BITMAP_FIELD_SIZE 64

// We set the max count to (2^32)/2 so we don't do something crazy via an error,
// this means max memory size is limited to 268,435,456 bytes (aka 256 megs)
#define VI_BITMAP_MAX_BIT_COUNT 2147483648

typedef struct {
	VI_BITMAP_FIELD_TYPE *fields;
	uint32_t field_count;
	uint64_t bit_count;
} vi_bitmap;

int vit_bitmap_tests( int argc, char *argv[] );
void vit_bitmap_run_test( vi_bitmap *b, uint64_t bit );
void vit_for_each_callback( uint64_t bit, void *data );

bool vi_bitmap_create( vi_bitmap *b, uint64_t size );
bool vi_bitmap_test( vi_bitmap *b, uint64_t bit_num );
bool vi_bitmap_set( vi_bitmap *b, uint64_t bit_num );
bool vi_bitmap_clear( vi_bitmap *b, uint64_t bit_num );
bool vi_bitmap_set_to( vi_bitmap *b, uint64_t bit_num, bool val );
void vi_bitmap_for_each_set( vi_bitmap *b, void (*func)(VI_BITMAP_FIELD_TYPE, void *), void *data );

#ifdef __cplusplus
}
#endif
#endif