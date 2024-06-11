#ifndef VIOS_SYSCALL_INCLUDED
#define VIOS_SYSCALL_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

size_t write( int fd, void *buff, size_t count );

#ifdef __cplusplus
}
#endif
#endif