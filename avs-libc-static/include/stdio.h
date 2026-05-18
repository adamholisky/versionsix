#ifndef STDIO_H__
#define STDIO_H__

#include <printf.h>
#include <stdarg.h>
#include <stddef.h>
#include <wctype.h> //TODO: eliminate need for this header here (refactor types?)
#include <stdbool.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#define __isoc_va_list va_list ap

typedef union _G_fpos64_t
{
	char __opaque[16];
	long long __lldata;
	double __align;
} fpos_t;

/*
 * Stdio buffers.
 */
struct __sbuf
{
	unsigned char* _base;
	int _size;
};

/*
 * struct __sFILE_fake is the start of a struct __sFILE, with only the
 * minimal fields allocated.  In __sinit() we really allocate the 3
 * standard streams, etc., and point away from this fake.
 */
//struct __sFILE_fake
//{
//	unsigned char* _p; /* current position in (some) buffer */
//	int _r; /* read space left for getc() */
//	int _w; /* write space left for putc() */
//	short _flags; /* flags, below; this FILE is free if 0 */
//	short _file; /* fileno, if Unix descriptor, else -1 */
//	struct __sbuf _bf; /* the buffer (at least 1 byte, if !NULL) */
//	int _lbfsize; /* 0 or -_bf._size, for inline putc */
//
//	struct _reent* _data;
//};

//#if !defined(__FILE_defined)
//typedef struct __sFILE_fake FILE;
//#define __FILE_defined
//#endif

#define mode_t unsigned int

#ifndef AVSOS_KERNEL
#include <sys/stat.h>

#undef mode_t
#else
	#include <fs.h>
#endif

typedef struct {
	int fd;
	int mode;
	long pos;
	int error_num;
	bool in_use;
	size_t size;
	char pathname[255];
	uint32_t pid;
	char *buf;
	size_t buf_size;
	char *rpos;
	char *rend;
	unsigned char *shend;
	off_t shlim, shcnt;
} FILE;

FILE *libc_internal_get_new_file_handle( void );

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

#define shcnt(f) ((f)->shcnt + ((f)->rpos - (f)->buf))
#define shlim(f, lim) __shlim((f), (lim))
#define shgetc(f) (((f)->rpos != (f)->shend) ? *(f)->rpos++ : __shgetc(f))
#define shunget(f) ((f)->shlim>=0 ? (void)(f)->rpos-- : (void)0)
#define sh_fromstring(f, s) ((f)->buf = (f)->rpos = (void *)(s), (f)->rend = (void*)-1)

size_t fread( void *ptr, size_t size, size_t nmemb, FILE *fh );
size_t fwrite( const void *ptr, size_t size, size_t nmemb, FILE *fh );
int fprintf(FILE* __restrict, const char* __restrict, ...);
int fstat( int fd, struct stat *statbuf );
int fputc(int c, FILE* fh );
int putc( int , FILE* fh );
char* fgets( char* s, int size, FILE* fh );
int fseek( FILE* fh, long offset, int whence );
int fgetc( FILE *stream );
int getc( FILE *stream );
int getchar( void );
int ungetc( int c, FILE *stream );
FILE* fopen(const char* __restrict, const char* __restrict);
FILE* freopen(const char* __restrict, const char* __restrict, FILE* __restrict);
int fclose(FILE*);
FILE* tmpfile(void);
void __shlim(FILE *f, off_t lim);
int __shgetc(FILE *f);
int __uflow(FILE *f);
long double __floatscan(FILE *, int, int);


int setvbuf( FILE *stream, char buf, int mode, size_t size );




// #pragma mark - Supported Functions -

/// Requires a definition of _putchar() for your platform
int putchar(int c);
int puts(const char*);

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
int asprintf(char**, const char*, ...);
int vasprintf(char**, const char*, __isoc_va_list);
#endif

// #pragma mark - Unsupported Functions -

#ifndef DISABLE_UNIMPLEMENTED_LIBC_APIS

long ftell(FILE*);
void rewind(FILE*);

int fgetpos(FILE* __restrict, fpos_t* __restrict);
int fsetpos(FILE*, const fpos_t*);

#if __STDC_VERSION__ < 201112L
char* gets(char*);
#endif

wchar_t* fgetws(wchar_t* __restrict, int, FILE* __restrict);
int fputws(const wchar_t* __restrict, FILE* __restrict);



wint_t fgetwc(FILE*);
wint_t getwc(FILE*);
wint_t getwchar(void);
wint_t ungetwc(wint_t, FILE*);

wint_t fputwc(wchar_t, FILE*);
wint_t putwc(wchar_t, FILE*);
wint_t putwchar(wchar_t);

char* tmpnam(char*);


int fwide(FILE*, int);

int fputs(const char* __restrict, FILE* __restrict);

FILE* fopen(const char* __restrict, const char* __restrict);
FILE* freopen(const char* __restrict, const char* __restrict, FILE* __restrict);
int fclose(FILE*);

int feof(FILE*);
int ferror(FILE*);
int fflush(FILE*);
void clearerr(FILE*);

int remove(const char*);
int rename(const char*, const char*);

void setbuf(FILE* __restrict, char* __restrict);

int scanf(const char* __restrict, ...);
int fscanf(FILE* __restrict, const char* __restrict, ...);
int sscanf(const char* __restrict, const char* __restrict, ...);
int vscanf(const char* __restrict, __isoc_va_list);
int vfscanf(FILE* __restrict, const char* __restrict, __isoc_va_list);
int vsscanf(const char* __restrict, const char* __restrict, __isoc_va_list);
int wscanf(const wchar_t* __restrict, ...);
int fwscanf(FILE* __restrict, const wchar_t* __restrict, ...);
int swscanf(const wchar_t* __restrict, const wchar_t* __restrict, ...);
int vwscanf(const wchar_t* __restrict, __isoc_va_list);
int vfwscanf(FILE* __restrict, const wchar_t* __restrict, __isoc_va_list);
int vswscanf(const wchar_t* __restrict, const wchar_t* __restrict, __isoc_va_list);

#ifndef vprintf
// vprintf is defined in printf.h
int vprintf(const char* __restrict, __isoc_va_list);
#endif

/// Unsupported printf variants

void perror(const char*);

int wprintf(const wchar_t* __restrict, ...);
int vfprintf(FILE* __restrict, const char* __restrict, __isoc_va_list);
int vsprintf(char* __restrict, const char* __restrict, __isoc_va_list);
int fwprintf(FILE* __restrict, const wchar_t* __restrict, ...);
int swprintf(wchar_t* __restrict, size_t, const wchar_t* __restrict, ...);
int vwprintf(const wchar_t* __restrict, __isoc_va_list);
int vfwprintf(FILE* __restrict, const wchar_t* __restrict, __isoc_va_list);
int vswprintf(wchar_t* __restrict, size_t, const wchar_t* __restrict, __isoc_va_list);
#endif

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // STDIO_H__
