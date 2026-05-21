#include <kernel_common.h>
#include <kmemory.h>
#include <process.h>

#include <wait_queue.h>

avs_list *all_queues;

/**
 * @brief Initalize wait queues
 * 
 */
void wq_initalize( void ) {
	all_queues = avs_list_init();
}

/**
 * @brief Create a new wait queue and add it to the system
 * 
 * @param name 
 * @param ready_func 
 * @return wait_queue* 
 */
wait_queue* wq_new( const char *name, wq_func ready_func ) {
	wait_queue *wq = kmalloc( sizeof(wait_queue) );

	kstrncpy( wq->name, name, 50 );
	wq->ready_func = ready_func;
	wq->queue = avs_list_init();

	return wq;
}

/**
 * @brief Add the given process to the wait queue with the associated data
 * 
 * @param wq 
 * @param pid 
 * @param wait_queue_data 
 * @return true 
 * @return false 
 */
bool wq_add( wait_queue *wq, pid_t pid, void *wait_queue_data ) {
	wait_queue_entry *wqe = kmalloc( sizeof(wait_queue_entry) );
	wqe->pid = pid;
	wqe->wqd = wait_queue_data;

	avs_list_enqueue( wq->queue, wqe );

	process_data *p = process_get_data_from_pid( pid );
	p->status = PROCESS_STATUS_WAITING;

	return true;
}

/**
 * @brief Make the queue ready
 * 
 * @param wq 
 */
void wq_make_ready( wait_queue *wq ) {
	wq->is_ready = true;
}

void wq_call_next_ready( wait_queue *wq ) {
	if( wq->queue->tail != NULL ) {
		wait_queue_entry *wqe = avs_list_dequeue( wq->queue );

		wq->ready_func( wq, wqe->pid, wqe->wqd );
	}
}