#ifndef VIOS_BOOTSTRAP_INCLUDED
#define VIOS_BOOTSTRAP_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef uint8_t byte;
typedef uint16_t word;

/* Outputs a byte to the specified hardware port */
static inline void outportb( uint32_t port, uint8_t value ) {
    __asm__ __volatile__ ("outb %%al,%%dx"::"d" (port), "a" (value));
}

/* Outputs a word to a port */
static inline void outportw( uint32_t port, uint32_t value ) {
    __asm__ __volatile__ ("outw %%ax,%%dx"::"d" (port), "a" (value));
}

/* gets a byte from a port */
static inline uint8_t inportb( uint32_t port ) {
    uint8_t value;
    __asm__ __volatile__ ("inb %w1,%b0" : "=a"(value) : "d"(port));
    return value;
}

static inline uint8_t inportw( uint32_t port ) {
    uint8_t value;
    __asm__ __volatile__ ("inw %%dx,%%ax" : "=a"(value) : "d"(port));
    return value;
}

static inline void out_port_long( uint16_t port, uint32_t value) {
    __asm__ __volatile__ ( "outl %%eax, %%dx" : : "d" (port), "a" (value) );
}

static inline uint32_t in_port_long( uint16_t port ) {
    uint32_t value;
    __asm__ __volatile__ ("inl %%dx, %%eax" : "=a"(value) : "dN"(port));
    return value;
}

#define set_bit(x,b) x | 1<<b
#define clear_bit(x,b) x ~ 1<<b 
#define flip_bit(x,b) x ^ 1<<b
#define test_bit(x,b) x & 1<<b
#define do_immediate_shutdown() outportb( 0xF4, 0x00 )

/* #define debugf( ... ) printf( __VA_ARGS__ )
#define log_entry_enter() debugf( "Enter\n" )
#define log_entry_exit() debugf( "Exit\n" );

#define dbA() debugf( "A" )
#define dbB() debugf( "B" )
#define dbC() debugf( "C" )
#define dbD() debugf( "D" )
#define dbE() debugf( "E" )
#define db1() debugf( "1" )
#define db2() debugf( "2" )
#define db3() debugf( "3" )
#define db4() debugf( "4" )
#define db5() debugf( "5" ) */

/* extern void sse_initalize( void ); */

#ifdef __cplusplus
void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void *p);
void operator delete[](void *p);
#endif

#ifdef __cplusplus
}

#endif

#endif