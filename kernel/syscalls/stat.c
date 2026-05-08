#include <kernel_common.h>
#include <syscall.h>
#include <process.h>
#include <vfs.h>

extern kernel_proc_data global_proc_data;

int statfs( const char *path, struct stat *statbuf ) {
	syscall_args args = {
		.arg_1 = path,
		.arg_2 = statbuf
	};

	return syscall( SYSCALL_STATFS, 2, &args );
}

int statfs_syscall_handler( registers **context, const char *path, struct stat *statbuf ) {
	return vfs_getattr( path, statbuf );
}
