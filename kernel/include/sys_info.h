#if !defined(SYSINFO_INCLUDED)
#define SYSINFO_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <kernel_common.h>

typedef struct {
	uint8_t		version;
	kinfo		kernel_info;
} sys_info;

#ifdef __cplusplus
}
#endif

#endif