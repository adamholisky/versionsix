#include <bootstrap.h>
#include "syscall.h"
#include "file.h"
#include "kmemory.h"

/**
 * @brief Read a line, terminated by return
 * 
 * @return char* 
 */
char * k_bs_line_read() {
	return 0;
}

extern "C" void term_put_char( char c ) {
    write( FD_STDOUT, &c, 1 );
}

extern "C" void __cxa_pure_virtual() {
   // Intentionally blanks
}

void *operator new(size_t size)
{
	return kmalloc(size);
}
 
void *operator new[](size_t size)
{
	return kmalloc(size);
}
 
void operator delete(void *p)
{
	kfree((uint64_t *)p);
}
 
void operator delete[](void *p)
{
	kfree((uint64_t *)p);
}