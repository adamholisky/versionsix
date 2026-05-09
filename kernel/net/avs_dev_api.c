#include <kernel_common.h>
#include <avs_dev_api.h>
#include <serial.h>
#include <stdlib.h>
#include <kmemory.h>

#define DEV_API_SERIAL_PORT 0x2F8 // COM2

static const char base64_tables[][65] = {
	[BASE64_STD] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",
	[BASE64_URLSAFE] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_",
	[BASE64_IMAP] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+,",
};

char api_call[255];

char json_size_start[] = { "{ \"cmd\": \"size\", \"file\": \"" };
char json_size_end[] = { ".exec\" }\n" };

char json_load_start[] = { "{ \"cmd\": \"load\", \"file\": \"" };
char json_load_end[] = { ".exec\" }\n" };

/**
 * @brief 
 * 
 * @param path 
 * @return uint64_t 
 */
uint64_t avs_dev_api_get_file_size( char *path ) {
	uint64_t exec_size = 0;
	char reply[255];

	memset( reply, 0, 255 );
	memset( api_call, 0, 255 );

	kstrcpy( api_call, json_size_start );
	kstrcat( api_call, path );
	kstrcat( api_call, json_size_end );

	printf( "Sending API: -->%s<--\n", api_call );

	avs_dev_api_send( api_call );
	printf( "Waiting.\n" );
	int reply_size = avs_dev_api_wait_for_reply( reply, 255 );

	exec_size = strtol( reply, NULL, 10 );
	
	return exec_size;
}

/**
 * @brief 
 * 
 * @param path 
 * @param data 
 * @return int 
 */
size_t avs_dev_api_load_file( char *path, void *data ) {
	size_t file_size_final = 0;

	char *b64_data = kmalloc( 1024 * 1024 );

	memset( api_call, 0, 255 );
	kstrcpy( api_call, json_load_start );
	kstrcat( api_call, path );
	kstrcat( api_call, json_load_end );

	printf( "Sending API: -->%s<--\n", api_call );
	avs_dev_api_send( api_call );
	printf( "Waiting.\n" );
	int b64_file_len = avs_dev_api_wait_for_reply( b64_data, 1024 * 1024 );

	printf( "--> size: %ld\n", b64_file_len );

	file_size_final = base64_decode( b64_data, b64_file_len, data, false, BASE64_STD );

	printf( "--> decoded size: %ld\n", file_size_final );

	kdebug_peek_at( data );

	kfree( b64_data );

	return file_size_final;
}

/**
 * @brief 
 * 
 * @param cmd 
 * @return int 
 */
int avs_dev_api_send( char *cmd ) {
	avs_dev_api_write_str_to_serial_port( cmd, kstrlen(cmd) );
}

/**
 * @brief 
 * 
 * @param s 
 * @param len 
 */
void avs_dev_api_write_str_to_serial_port( char *s, int len ) {
	printf( "Writing: len=%d\n", len );

	for( int i = 0; i < len; i++ ) {
		while((inportb(DEV_API_SERIAL_PORT + 5) & 0x20) == 0) {
			;
		}

		outportb( DEV_API_SERIAL_PORT, *s++ );
	}

	printf( "Done writing\n" );
}

/**
 * @brief 
 * 
 * @param reply 
 * @param max_size 
 * @return int 
 */
int avs_dev_api_wait_for_reply( char *reply, size_t max_size ) {
	int bytes_recvd = 0;
	char c = 0;

	// Add timeout check
	do {
		c = serial_read_port( DEV_API_SERIAL_PORT );

		if( c == '\n' ) {
			break;
		} else {
			reply[bytes_recvd++] = c;
		}
	} while( bytes_recvd < max_size );

	return bytes_recvd;
}

// The base64 stuff is from the linux kernel

#define INIT_1(v, ch_62, ch_63) \
	[v] = (v) >= 'A' && (v) <= 'Z' ? (v) - 'A' \
		: (v) >= 'a' && (v) <= 'z' ? (v) - 'a' + 26 \
		: (v) >= '0' && (v) <= '9' ? (v) - '0' + 52 \
		: (v) == (ch_62) ? 62 : (v) == (ch_63) ? 63 : -1

#define INIT_2(v, ...) INIT_1(v, __VA_ARGS__), INIT_1((v) + 1, __VA_ARGS__)
#define INIT_4(v, ...) INIT_2(v, __VA_ARGS__), INIT_2((v) + 2, __VA_ARGS__)
#define INIT_8(v, ...) INIT_4(v, __VA_ARGS__), INIT_4((v) + 4, __VA_ARGS__)
#define INIT_16(v, ...) INIT_8(v, __VA_ARGS__), INIT_8((v) + 8, __VA_ARGS__)
#define INIT_32(v, ...) INIT_16(v, __VA_ARGS__), INIT_16((v) + 16, __VA_ARGS__)

#define BASE64_REV_INIT(ch_62, ch_63) { \
	[0 ... 0x1f] = -1, \
	INIT_32(0x20, ch_62, ch_63), \
	INIT_32(0x40, ch_62, ch_63), \
	INIT_32(0x60, ch_62, ch_63), \
	[0x80 ... 0xff] = -1 }

static const int8_t base64_rev_maps[][256] = {
	[BASE64_STD] = BASE64_REV_INIT('+', '/'),
	[BASE64_URLSAFE] = BASE64_REV_INIT('-', '_'),
	[BASE64_IMAP] = BASE64_REV_INIT('+', ',')
};

/**
 * base64_decode() - Base64-decode a string
 * @src: the string to decode.  Doesn't need to be NUL-terminated.
 * @srclen: the length of @src in bytes
 * @dst: (output) the decoded binary data
 * @padding: whether to append '=' padding characters
 * @variant: which base64 variant to use
 *
 * Decodes a string using the selected Base64 variant.
 *
 * Return: the length of the resulting decoded binary data in bytes,
 *	   or -1 if the string isn't a valid Base64 string.
 */
int base64_decode(const char *src, int srclen, uint8_t *dst, bool padding, enum base64_variant variant)
{
	uint8_t *bp = dst;
	int8_t input[4];
	int32_t val;
	const uint8_t *s = (const uint8_t *)src;
	const int8_t *base64_rev_tables = base64_rev_maps[variant];

	while (srclen >= 4) {
		input[0] = base64_rev_tables[s[0]];
		input[1] = base64_rev_tables[s[1]];
		input[2] = base64_rev_tables[s[2]];
		input[3] = base64_rev_tables[s[3]];

		val = input[0] << 18 | input[1] << 12 | input[2] << 6 | input[3];

		if (val < 0) {
			if (!padding || srclen != 4 || s[3] != '=')
				return -1;
			padding = 0;
			srclen = s[2] == '=' ? 2 : 3;
			break;
		}

		*bp++ = val >> 16;
		*bp++ = val >> 8;
		*bp++ = val;

		s += 4;
		srclen -= 4;
	}

	if (!srclen)
		return bp - dst;
	if (padding || srclen == 1)
		return -1;

	val = (base64_rev_tables[s[0]] << 12) | (base64_rev_tables[s[1]] << 6);
	*bp++ = val >> 10;

	if (srclen == 2) {
		if (val & 0x800003ff)
			return -1;
	} else {
		val |= base64_rev_tables[s[2]];
		if (val & 0x80000003)
			return -1;
		*bp++ = val >> 2;
	}
	return bp - dst;
}