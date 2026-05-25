#include <stdio.h>
#include <kmemory.h>
#include <typeinfo>
#include <exception>
#include <kernel_common.h>

/******************************************/
/* C++ Support routines to keep           */
/******************************************/

// Begin magic. Got help with this part... it's magic to me.

static uint8_t _kernel_tls_block[256] __attribute__((aligned(16)));

static void setup_kernel_tls(void) {
    reinterpret_cast<uint64_t*>(_kernel_tls_block)[5] = 0xDEADBEEFDEADBEEFULL;
    uint64_t base = reinterpret_cast<uint64_t>(_kernel_tls_block);
    uint32_t lo = static_cast<uint32_t>(base & 0xFFFFFFFF);
    uint32_t hi = static_cast<uint32_t>(base >> 32);
    __asm__ volatile("wrmsr" :: "c"(0xC0000100U), "a"(lo), "d"(hi));
}

extern "C" uint8_t __eh_frame_start[];
extern "C" uint8_t __eh_frame_end[];
extern "C" void __register_frame_info(const void *begin, void *ob);

#define MAX_EH_CHUNKS 8
static uint8_t _kernel_eh_frame_ob[MAX_EH_CHUNKS][128] __attribute__((aligned(8)));

static void setup_exception_handling(void) {
    uint8_t *p   = __eh_frame_start;
    uint8_t *end = __eh_frame_end;
    int chunk = 0;

    while (p < end && chunk < MAX_EH_CHUNKS) {
        while (p < end && *reinterpret_cast<uint32_t*>(p) == 0)
            p += 4;
        if (p >= end) break;

        __builtin_memset(_kernel_eh_frame_ob[chunk], 0, sizeof(_kernel_eh_frame_ob[chunk]));
        __register_frame_info(p, _kernel_eh_frame_ob[chunk]);
        chunk++;

        while (p < end) {
            uint32_t len = *reinterpret_cast<uint32_t*>(p);
            if (len == 0) { p += 4; break; }
            p += 4 + len;
        }
    }
}



namespace __cxxabiv1 {
    struct __cxa_exception;
    struct __cxa_eh_globals {
        __cxa_exception *caughtExceptions;
        unsigned int uncaughtExceptions;
    };
}

static __cxxabiv1::__cxa_eh_globals _kernel_eh_globals;

extern "C" __cxxabiv1::__cxa_eh_globals* __cxa_get_globals() noexcept {
    return &_kernel_eh_globals;
}

extern "C" __cxxabiv1::__cxa_eh_globals* __cxa_get_globals_fast() noexcept {
    return &_kernel_eh_globals;
}

// End magic.

extern "C" void __cxa_pure_virtual() {

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
    kfree(p);
}

void operator delete[](void *p)
{
    kfree(p);
}

extern "C" void __stack_chk_fail(void) {
	printf("__stack_chk_failed" );
    
	while( true ) {
		;
	}
}

/******************************************/
/* END C++ Support routines to keep       */
/******************************************/

extern "C" void do_cpp_tests( void );
void rtti_tests( void );
void exceptions_tests( void );

class MyBase {
	public:
	virtual void do_a_virt_func( void ) {
		 //printf( "in a virt func\n" );
	}
};

class MyClass : public MyBase {
	public:
	MyClass( void );
};

class MySecondClass {
	public:
	virtual void some_func( void ) {
		;
	}

	MySecondClass( void ) {
		//printf( "Const of second class.\n" );
	}
};

extern "C" void do_cpp_tests( void ) {
	//printf( "In C++ tests.\n" );

	setup_kernel_tls();
	setup_exception_handling();

	rtti_tests();
	exceptions_tests();

	//printf( "Out C++ tests.\n" );
}

void rtti_tests( void ) {
	bool tests_passed = true;

	MyClass *mc = new MyClass();

	MyBase *b = mc;

	//printf( "typeid b: %s\n", typeid(b).name() );
	//printf( "typeid mc: %s\n", typeid(mc).name() );

	MySecondClass *sc = new MySecondClass();

	//printf( "typeid sc: %s\n", typeid(*sc).name() );

	if( typeid(*b) == typeid(MyClass) ) {
		//printf( "it's a MyClass type.\n" );
	} else {
		//printf( "It's NOT a MyClass type.\n" );
		tests_passed = false;
	}

	if( typeid(*b) == typeid(MySecondClass) ) {
		//printf( "it's a MySeocondClass type.\n" );
		tests_passed = false;
	} else {
		//printf( "It's NOT a MySecondClass type.\n" );
	}

	if( tests_passed ) {
		debugf( "C++ RTTI tests passed.\n" );
	} else {
		debugf( "C++ RTTI tests failed.\n" );
	}
}

MyClass::MyClass( void ) {
	//printf( "In constructor.\n" );
}

void exceptions_tests( void ) {
	//printf( "Start excp test\n" );

	bool tests_passed = false;

	try {
		throw( "My exception is throwing now" );
	} catch( const char *e ) {
		//printf( "Caught runtime error: %s\n", e );
		tests_passed = true;
	}

	if( tests_passed ) {
		debugf( "C++ Exceptions test passed.\n" );
	} else {
		debugf( "C++ Exceptions test failed.\n" );
	}

	//printf( "End excp test\n" );
}