#include <bootstrap.h>
#include "syscall.h"
#include "file.h"
#include "kmemory.h"
#include <kernel_common.h>
#include <ksymbols.h>
#include <device.h>

bool profiling_ok = false;
bool profiling_first_call = false;

/**
 * @brief Read a line, terminated by return
 * 
 * @return char* 
 */
char * k_bs_line_read() {
	return 0;
}

char kgetc( void ) {
	char c;

	device *dev_std_in = device_get_major_minor_device( "stdin", "0" );

	dev_std_in->read(0, &c, 1, 1 );

	if( c == 13 ) {
		c = '\n';
	}

	return c;
}

void kputc( char c ) {

}

void term_put_char( char c ) {
    write( FD_STDOUT, &c, 1 );
}

void delay( uint32_t count ) {
	for( uint32_t x = 0; x < count; x++ ) {
		__asm__ __volatile__ ( "nop" );
	}
}

/* int kstrlen( char *s ) {
	int len = 0;

	while( *(s++) ) {
		len++;
	}

	return len;
} */

/**
 * @brief Kernel debug strlen
 * 
 * Pulled from linux lib/string.c
 * 
 * @param s 
 * @return size_t 
 */
size_t kstrlen(const char *s) {
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}


int vios_fprintf( int dest, char *message, ... ) {
	char buff[1024];
	va_list args;
	int len = 0;

	va_start( args, message );
	len = vsnprintf( buff, 1024, message, args ); 
	va_end( args );

	if( dest == vios_stderr ) {
		write( FD_STDERR, buff, len );
	} else if( dest == vios_stdout ) {
		write( FD_STDOUT, buff, len );
	}
}

/**
 * @brief strcpy
 * 
 * Pulled from linux kerenl lib/string.c
 * 
 * @param dest 
 * @param src 
 * @return char* 
 */
char *kstrcpy(char *dest, const char *src)
{
	char *tmp = dest;

	while ((*dest++ = *src++) != '\0')
		/* nothing */;
	return tmp;
}

/**
 * @brief strcat
 * 
 * Pulled from linux kernel lib/string.c
 * 
 * @param dest 
 * @param src 
 * @return char* 
 */
char *kstrcat(char *dest, const char *src)
{
	char *tmp = dest;

	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;
	return tmp;
}


/**
 * @brief strncat
 * 
 * Pulled form linux kernel lib/string.c
 * 
 * @param dest 
 * @param src 
 * @param count 
 * @return char* 
 */
char *kstrncat(char *dest, const char *src, size_t count)
{
	char *tmp = dest;

	if (count) {
		while (*dest)
			dest++;
		while ((*dest++ = *src++) != 0) {
			if (--count == 0) {
				*dest = '\0';
				break;
			}
		}
	}
	return tmp;
}

/**
 * @brief kstrncpy
 * 
 * Pulled from linux kernel lib/string.c
 * 
 * @param dest 
 * @param src 
 * @param count 
 * @return char* 
 */
char *kstrncpy(char *dest, const char *src, size_t count)
{
	char *tmp = dest;

	while (count) {
		if ((*tmp = *src) != 0)
			src++;
		tmp++;
		count--;
	}
	return dest;
}

/**
 * @brief kstrcmp
 * 
 * Pulled from linux kernel lib/string.c
 */
int kstrcmp(const char *cs, const char *ct)
{
	unsigned char c1, c2;

	while (1) {
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
	}
	return 0;
}

#ifdef VIOS_ENABLE_PROFILING
extern uint64_t system_count;

void profiling_initalize( void ) {
	profiling_ok = true;
	profiling_first_call = true;
}

void __cyg_profile_func_enter (void *this_fn, void *call_site) {
	if( profiling_ok ) {
		kernel_symbols_inc_count( (uint64_t)this_fn ); 
		kernel_symbols_set_start( (uint64_t)this_fn, system_count );
		/* write( FD_STDERR, "Called: ", sizeof("Called: ") - 1 );
		write( FD_STDERR, name, kstrlen(name) );
		write( FD_STDERR, "\n", 1 ); */
	}

	return;
}

void __cyg_profile_func_exit  (void *this_fn, void *call_site) {
	if( profiling_ok ) {
		if( profiling_first_call == true ) {
			profiling_first_call = false;
			return;
		}

		// Record exit time
		kernel_symbols_set_time( (uint64_t)this_fn, system_count );
	}

	return;
}
#endif