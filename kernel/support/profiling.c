#include <kernel_common.h>
#include <bootstrap.h>
#include <ksymbols.h>
#include <profiling.h>

bool profiling_ok = false;
bool profiling_first_call = false;

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