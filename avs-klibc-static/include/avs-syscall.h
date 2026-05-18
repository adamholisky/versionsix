#ifndef VIOS_SYSCALL_INCLUDED
#define VIOS_SYSCALL_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define SYSCALL_READ 0
#define SYSCALL_WRITE 1
#define SYSCALL_OPEN 2
#define SYSCALL_CLOSE 3
#define SYSCALL_SCHED_YIELD 4
#define SYSCALL_EXECVE 5
#define SYSCALL_FORK 6
#define SYSCALL_EXIT 7
#define SYSCALL_WAIT 8
#define SYSCALL_EXECM 9
#define SYSCALL_STATFS 10

typedef uint16_t umode_t;
typedef uint32_t pid_t;

typedef struct {
	uint64_t	arg_1;
	uint64_t	arg_2;
	uint64_t	arg_3;
	uint64_t	arg_4;
	uint64_t	arg_5;
	uint64_t	arg_6;
} syscall_args;

// User facing
uint64_t syscall( uint64_t call_num, uint8_t num_args, syscall_args *args );
void libc_internal_initalize( void );
FILE* libc_internal_get_new_file_handle( void );
FILE* libc_internal_get_file_from_fd( int fd );

// Glue

int close( int fd );
int execve( char *path, char *argv[], char *envp[] );
int execm( void *exec_data, char *argv[], char *envp[] );
void exit( int status );
int open( const char *path, int flags );
pid_t wait( pid_t pid );
size_t write( int fd, void *buff, size_t count );
size_t read( int fd, void *buf, size_t count );
int statfs( const char *path, struct stat *statbuf );

#ifdef __cplusplus
}
#endif
#endif