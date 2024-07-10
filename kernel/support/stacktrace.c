#include <kernel_common.h>
#include <stacktrace.h>
#include <ksymbols.h>

void stacktrace_out_for_rbp( uint64_t rbp, bool use_stdout, bool use_stderr, uint8_t num_spaces ) {
	struct stackframe *sf = (struct stackframe *)rbp;
		
	for( int i = 0; i < 15; i++ ) {
		if( use_stderr ) {
			for( int spaces = 0; spaces < num_spaces; spaces++ ) {
				debugf_raw( " " );
			}
			debugf_raw( "[%d] 0x%016llX %s\n", i, sf->rip, kernel_symbols_get_function_name_at(sf->rip) );
		}

		if( use_stdout ) {
			for( int spaces = 0; spaces < num_spaces; spaces++ ) {
				printf( " " );
			}
			printf( "[%d] 0x%016llX %s\n", i, sf->rip, kernel_symbols_get_function_name_at(sf->rip) );
		}
		
		sf = sf->rbp;

		if( sf != NULL ) {
			if( sf->rbp == 0 ) {
				i = 15;
			}
		} else {
			i = 15;
		}
	}
}