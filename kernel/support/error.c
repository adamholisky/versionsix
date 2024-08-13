#include <kernel_common.h>
#include <errno.h>

char *error_get_str( int16_t error_no ) {
	int16_t err = error_no;

	if( err < -1 ) {
		err = err * -1;
	}

	return error_strings[err];
}