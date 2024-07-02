#ifndef VIOS_SYSCALL_INCLUDED
#define VIOS_SYSCALL_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <interrupt.h>
#include <stdbool.h>

#define SYSCALL_READ 0
#define SYSCALL_WRITE 1
#define SYSCALL_OPEN 2
#define SYSCALL_CLOSE 3
#define SYSCALL_SCHED_YIELD 4

typedef struct {
	uint64_t	arg_1;
	uint64_t	arg_2;
	uint64_t	arg_3;
	uint64_t	arg_4;
	uint64_t	arg_5;
	uint64_t	arg_6;
} syscall_args;

void syscall_initalize( void );
uint64_t syscall( uint64_t call_num, uint8_t num_args, syscall_args *args );
void syscall_handler( registers **_context );

size_t write( int fd, void *buff, size_t count );

#ifdef __cplusplus
}
#endif
#endif