#ifndef UNISTD_HEADER
#define UNISTD_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#define STDERR_FILENO 2
#define STDIN_FILENO 0
#define STDOUT_FILENO 1

int isatty( int fd );

#ifdef __cplusplus
}
#endif

#endif