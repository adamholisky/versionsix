#include <stdio.h>
#include <kmemory.h>
#include <typeinfo>
#include <stdexcept>
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
		printf( "in a virt func\n" );
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
		printf( "Const of second class.\n" );
	}
};

extern "C" void do_cpp_tests( void ) {
	printf( "In C++ tests.\n" );

	rtti_tests();
	exceptions_tests();

	printf( "Out C++ tests.\n" );
}

void rtti_tests( void ) {
	MyClass *mc = new MyClass();

	MyBase *b = mc;

	printf( "typeid b: %s\n", typeid(b).name() );
	printf( "typeid mc: %s\n", typeid(mc).name() );

	MySecondClass *sc = new MySecondClass();

	printf( "typeid sc: %s\n", typeid(*sc).name() );

	if( typeid(*b) == typeid(MyClass) ) {
		printf( "it's a MyClass type.\n" );
	} else {
		printf( "It's NOT a MyClass type.\n" );
	}

	if( typeid(*b) == typeid(MySecondClass) ) {
		printf( "it's a MySeocondClass type.\n" );
	} else {
		printf( "It's NOT a MySecondClass type.\n" );
	}
}

MyClass::MyClass( void ) {
	printf( "In constructor.\n" );
}

void exceptions_tests( void ) {
	try {
		throw( "My exception is throwing no" );
	} catch( std::runtime_error &r ) {
		printf( "Caught runtime error: %s\n", r.what() );
	}
}