#ifndef VIOS_HASH_INCLUDED
#define VIOS_HASH_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include "vit.h"

#define vi_hash qhashmurmur3_32
#define vi_hash_murmur3 qhashmurmur_3_32
#define vi_hash_murmur3_128 qhashmurmur3_128

uint32_t qhashmurmur3_32(const void *data, size_t nbytes);
bool qhashmurmur3_128(const void *data, size_t nbytes, void *retbuf);

#ifdef __cplusplus
}
#endif
#endif