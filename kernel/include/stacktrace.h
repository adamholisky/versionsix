#ifndef VIOS_STACKTRACE_INCLUDED
#define VIOS_STACKTRACE_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

struct stackframe {
	struct stackframe *rbp;
	uint64_t rip;
};

void stacktrace_out_for_rbp( uint64_t rbp, bool use_stdout, bool use_stderr, uint8_t num_spaces );

#ifdef __cplusplus
}
#endif
#endif