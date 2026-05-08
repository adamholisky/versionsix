#ifndef __SIGNAL_H_
#define __SIGNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*sighandler_t)(int);

sighandler_t signal( int signum, sighandler_t handler );

#ifdef __cplusplus
}
#endif

#endif
