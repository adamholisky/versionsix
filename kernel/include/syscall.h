#ifndef VIOS_SYSCALL_INCLUDED
#define VIOS_SYSCALL_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <interrupt.h>
#include <stdbool.h>
#include <process.h>

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

typedef uint16_t umode_t;

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

// Kernel facing
void syscall_initalize( void );
void syscall_handler( registers **_context );
size_t kwrite( int fd, void *buff, size_t count );

// Glue

int close( int fd );
int execve( char *path, char *argv[], char *envp[] );
int execm( void *exec_data, char *argv[], char *envp[] );
void exit( int status );
int open( const char *path, int flags );
pid_t wait( pid_t pid );
size_t write( int fd, void *buff, size_t count );
size_t read( int fd, void *buf, size_t count );

// Handlers
int close_syscall_handler( registers **context, int fd );
int execve_syscall_handler( registers **_context, char *path, char *argv[], char *envp[] );
int execm_syscall_handler( registers **_context, void *exec_data, char *argv[], char *envp[] );
void exit_syscall_handler( registers **context, int return_code );
int open_syscall_handler( registers **context, const char *path, int flags, umode_t mode  );
int wait_syscall_handler( registers **context, pid_t pid );
int read_syscall_handler( registers **context, int fd, void *buf, size_t count );
void write_syscall_handler( registers **context, int fd, void *buff, size_t count );

#ifdef __cplusplus
}
#endif
#endif