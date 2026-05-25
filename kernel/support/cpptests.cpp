#include <stdio.h>
#include <kmemory.h>
#include <typeinfo>
#include <kernel_common.h>

/******************************************/
/* C++ Support routines to keep           */
/******************************************/


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

inline void *operator new(size_t, void *p)     throw() { return p; }
inline void *operator new[](size_t, void *p)   throw() { return p; }
inline void  operator delete  (void *, void *) throw() { };
inline void  operator delete[](void *, void *) throw() { };

extern "C" void __stack_chk_fail(void) {
	printf( "__stack_chk_failed" );
    
	while( true ) {
		;
	}
}

/******************************************/
/* END C++ Support routines to keep       */
/******************************************/

extern "C" void do_cpp_tests( void );

class MyBase {
	public:
	virtual void do_a_virt_func( void ) {
		printf( "in a virt func\n" );
	}
};

class MyClass : public MyBase {
	public:
	MyClass( void );
};

extern "C" void do_cpp_tests( void ) {
	printf( "In C++ tests.\n" );

	MyClass *mc = new MyClass();

	MyBase *b = mc;

	if( typeid(b) == typeid(MyClass) ) {
		printf( "it's a MyClass type.\n" );
	} else {
		printf( "It's NOT a MyClass type.\n" );
	}
}

MyClass::MyClass( void ) {
	printf( "In constructor.\n" );
}

