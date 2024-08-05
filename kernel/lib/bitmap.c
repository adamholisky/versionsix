#include "vit.h"



int vit_bitmap_tests( int argc, char *argv[] ) {
	vdf( "in bitmap tests\n" );

	vdf( "sizeof(uint32_t): %d\n", sizeof(uint32_t) );
	vdf( "sizeof(uint64_t): %d\n", sizeof(uint64_t) );
	vdf( "sizeof(size_t): %d\n", sizeof(size_t) );

	vi_bitmap *bitmap = vmalloc( sizeof(vi_bitmap) );

	if( !vi_bitmap_create( bitmap, 512*512 ) ) {
		vdf( "Bitmap creating failed.\n" );
		return 1;
	}

	vit_bitmap_run_test( bitmap, 0 );
	vit_bitmap_run_test( bitmap, 3 );
	vit_bitmap_run_test( bitmap, 63 );
	vit_bitmap_run_test( bitmap, 64 );
	vit_bitmap_run_test( bitmap, 65 );

	vi_bitmap_set( bitmap, 512*512 );
	vi_bitmap_set( bitmap, 512*512 + 1 );
	vi_bitmap_set( bitmap, 10 );
	vi_bitmap_set( bitmap, 64 );
	vi_bitmap_set( bitmap, 65 );
	vi_bitmap_set( bitmap, 66 );
	vi_bitmap_set( bitmap, 0 );
	vi_bitmap_set( bitmap, 1 );
	//vi_bitmap_set( bitmap, 42 );
	//vit_bitmap_run_test( bitmap, 512*512*4 + 1 );

	vi_bitmap_for_each_set( bitmap, vit_for_each_callback, NULL );

	return 0;
}

/**
 * @brief Run dev set/clear tests for given bit
 * 
 * @param b bit map struct
 * @param bit bit number
 */
void vit_bitmap_run_test( vi_bitmap *b, uint64_t bit ) {
	vdf( "testing bit number %d\n", bit );
	vdf( "    started:   bit %d: %d\n", bit, vi_bitmap_test( b, bit ) );
	vi_bitmap_set( b, bit );
	vdf( "    after set: bit %d: %d\n", bit, vi_bitmap_test( b, bit ) );
	vi_bitmap_clear( b, bit );
	vdf( "    after clr: bit %d: %d\n", bit, vi_bitmap_test( b, bit ) );
}

void vit_for_each_callback( uint64_t bit, void *data ) {
	vdf( "Bit #%ld is set.\n", bit );
}

/**
 * @brief Create a bitmap of the given size
 * 
 * @param size 1 through (max of size_t) bit fields. NOT 0-based.
 * @return bool true if successful, false otherwise
 */
bool vi_bitmap_create( vi_bitmap *b, uint64_t size ) {
	if( b == NULL ) {
		vdf( "vi_bitmap *b is NULL. Failing.\n" );
		return false;
	}

	if( size < 1 ) {
		vdf( "Size must be 1 or greater. Failing.\n" );
		return false;
	}

	if( size >= VI_BITMAP_MAX_BIT_COUNT ) {
		vdf( "Size must be under %ld, got %ld. Failing.\n", VI_BITMAP_MAX_BIT_COUNT, size );
		return false;
	}

	b->field_count = size / VI_BITMAP_FIELD_SIZE;

	if( size % VI_BITMAP_FIELD_SIZE != 0 ) {
		b->field_count++;
	}

	b->fields = vmalloc( sizeof(VI_BITMAP_FIELD_TYPE) * b->field_count );
	if( b->fields == NULL ) {
		vdf( "b->fields is NULL. Failing.\n" );
		return false;
	}
	b->bit_count = size;

	memset( b->fields, 0, b->field_count * (VI_BITMAP_FIELD_SIZE/8) );

	vdf( "size: %ld    field_count: %ld\n", size, b->field_count );

	return true;
}

/**
 * @brief Tests the state of the given bit
 * 
 * @param b Pointer to the bitmap struct
 * @param bit_num bit number to test
 * @return true Bit is set
 * @return false Bit is unset
 */
bool vi_bitmap_test( vi_bitmap *b, uint64_t bit_num ) {
	uint32_t field = bit_num / VI_BITMAP_FIELD_SIZE;
	uint64_t bit = bit_num % VI_BITMAP_FIELD_SIZE;

	if( b == NULL ) {
		vdf( "vi_bitmap *b is NULL. Failing.\n" );
		return false;
	}

	if( bit_num > b->bit_count ) {
		vdf( "Bit number exceeded allocated size. bit_num = %d, max = %d.\n", bit_num, b->bit_count );
		return false;
	}

	return (b->fields[field] & (1UL<<bit)) >= 1 ? true : false;
}

/**
 * @brief Sets the given bit
 * 
 * @param b Pointer to the bitmap struct
 * @param bit_num bit number to set
 * @return true Success
 * @return false Failure
 */
bool vi_bitmap_set( vi_bitmap *b, uint64_t bit_num ) {
	return vi_bitmap_set_to( b, bit_num, true );
}

/**
 * @brief Clears the given bit
 * 
 * @param b Pointer to the bitmap struct
 * @param bit_num bit number to clear
 * @return true Success
 * @return false Failure
 */
bool vi_bitmap_clear( vi_bitmap *b, uint64_t bit_num ) {
	return vi_bitmap_set_to( b, bit_num, false );
}

/**
 * @brief Sets a bit to the given state
 * 
 * @param b Pointer to the bitmap struct
 * @param bit_num bit number to set
 * @param val true (1) or false (0)
 * @return true Success
 * @return false Failure
 */
bool vi_bitmap_set_to( vi_bitmap *b, uint64_t bit_num, bool val ) {
	uint32_t field = bit_num / VI_BITMAP_FIELD_SIZE;
	uint64_t bit = bit_num % VI_BITMAP_FIELD_SIZE;

	if( b == NULL ) {
		vdf( "vi_bitmap *b is NULL. Failing.\n" );
		return false;
	}

	if( bit_num > b->bit_count ) {
		vdf( "Bit number exceeded allocated size. bit_num = %d, max = %d.\n", bit_num, b->bit_count );
		return false;
	}

	// intentionally omitting field size check for speed sake, given b ought to be configured right by now
	
	if( val ) {
		// Set bit
		b->fields[field] = b->fields[field] | (1UL<<bit);
	} else {
		// Clear bit
		b->fields[field] = b->fields[field] & (0UL<<bit);
	}

	//vdf( "setting field: %ld    bit: %d    set: %d (%llX)    field data: 0x%llX\n", field, bit, val, (1UL<<bit),b->fields[field] );
	return true;
}


void vi_bitmap_for_each_set( vi_bitmap *b, void (*func)(VI_BITMAP_FIELD_TYPE, void *), void *data ) {
	if( b == NULL ) {
		vdf( "vi_bitmap *b is NULL. Failing.\n" );
		return;
	}

	for( uint64_t i = 0; i < b->bit_count; i++ ) {
		if( vi_bitmap_test(b,i) ) {
			func( i, data );
		}
	}
}