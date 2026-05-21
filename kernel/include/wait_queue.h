#if !defined(WAIT_QUEUE_H_INCLUDED)
#define WAIT_QUEUE_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <ksymbols.h>
#include <lib/list.h>
#include <interrupt.h>
#include <process.h>

typedef struct wait_queue wait_queue;

typedef int (*wq_func)(wait_queue *queue, pid_t pid, void *wait_queue_data );

typedef struct {
	pid_t pid;
	void *wqd;
} wait_queue_entry;

typedef struct wait_queue {
	avs_list *queue;
	char name[50];
	bool is_ready;

	wq_func ready_func;
};

void wq_initalize( void );
wait_queue* wq_new( const char* name, wq_func ready_func );
bool wq_add( wait_queue *wq, pid_t pid, void *wait_queue_data );
void wq_make_ready( wait_queue* wq );
void wq_call_next_ready( wait_queue *wq );

#ifdef __cplusplus
}
#endif

#endif