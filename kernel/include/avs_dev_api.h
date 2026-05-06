#if !defined(AVS_DEV_API_INCLUDED)
#define AVS_DEV_API_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// The base64 stuff is from the linux kernel

enum base64_variant {
	BASE64_STD,       /* RFC 4648 (standard) */
	BASE64_URLSAFE,   /* RFC 4648 (base64url) */
	BASE64_IMAP,      /* RFC 3501 */
};

uint64_t avs_dev_api_get_file_size( char *path );
size_t avs_dev_api_load_file( char *path, void *data );
int avs_dev_api_send( char *cmd );
void avs_dev_api_write_str_to_serial_port( char *s, int len );
int avs_dev_api_wait_for_reply( char *reply, size_t max_size );

int base64_encode(const uint8_t *src, int len, char *dst, bool padding, enum base64_variant variant);
int base64_decode(const char *src, int len, uint8_t *dst, bool padding, enum base64_variant variant);



#ifdef __cplusplus
}
#endif

#endif