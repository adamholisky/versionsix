/**
 * @file list.h
 * @author Adam Holisky (adam.holisky@gmail.com)
 * @brief List header file
 * @version 0.1
 * @date 2026-03-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef AVSUL_LIST_H
#define AVSUL_LIST_H
#ifdef __cplusplus
extern "C" {
#endif

#include "node.h"

#define avs_allocate(x) kmalloc(x)
#define avs_free(x) kfree(x)

typedef struct {
    avs_node *head;
    avs_node *tail;
    int size;
} avs_list;

avs_list* avs_list_init( void );
avs_list* avs_list_append( avs_list *list, void *data );
avs_list* avs_list_prepend( avs_list *list, void *data );
avs_list* avs_list_insert_before(avs_list *list, avs_node *node, void *data);
avs_list* avs_list_insert_after(avs_list *list, avs_node *node, void *data);
avs_list* avs_list_push(avs_list *list, void *data);
void* avs_list_pop(avs_list *list);
void* avs_list_at_index_data(avs_list *list, int index);
avs_node* avs_list_at_index_node( avs_list *list, int index );
avs_list* avs_list_free( avs_list *list, avs_node *node );
avs_node* avs_list_remove( avs_list *list, avs_node *node );
void avs_list_empty( avs_list *list );

avs_node* avs_list_find_data( avs_list *list, void *data_to_find, int (*comparison_callback)(void *, void *) );
void avs_list_for_each( avs_list *list, void (*for_each_callback)(avs_node *) );
avs_list* avs_list_enqueue( avs_list *list, void *data );
void* avs_list_dequeue( avs_list *list );

#ifdef __cplusplus
}
#endif
#endif