#include <kernel_common.h>
#include <syscall.h>

size_t read( int fd, void *buf, size_t count ) {
	syscall_args args = {
		.arg_1 = fd,
		.arg_2 = buf,
		.arg_3 = count
	};

	return syscall( SYSCALL_READ, 3, &args );
}


int read_syscall_handler( registers **context, int fd, void *buf, size_t count ) {

}