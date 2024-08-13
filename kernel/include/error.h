#ifndef VIOS_ERROR_INCLUDED
#define VIOS_ERROR_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define ERR_NONE 0
#define ERR_UNKNOWN -1
#define ERR_STAT -2
#define ERR_INVALID_ELF_TYPE -3


static const char *error_strings[] = {
	"None",
	"Unknown",
	"Stat",
	"Invlaid ELF type"
};

char *error_get_str( int16_t error_no );


#ifdef __cplusplus
}
#endif
#endif