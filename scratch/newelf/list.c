/**
 * @file list.c
 * @author Adam Holisky (adam.holisky@gmail.com)
 * @brief A simple linked list implementation
 * @version 0.1
 * @date 2026-03-09
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "list.h"

#define avs_allocate malloc
#define avs_free free

/**
 * @brief Initalize an AVS list
 * 
 * @return avs_list* Pointer to the AVS list, fully allocated. NULL if failure.
 */
avs_list* avs_list_init( void ) {    
    //avs_init();

    avs_list *list = avs_allocate( sizeof(avs_list) );

    if( list == NULL ) {
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;

    return list;
}

/**
 * @brief Adds a new element at the end of the list
 * 
 * @param list Pointer ot the avs_list
 * @param data Pointer to the data to add to the list 
 * @return avs_list* NULL if failed, otherwise pointer to the avs_list
 */
avs_list* avs_list_append( avs_list *list, void *data ) {
    /* avs_node *node = avs_allocate( sizeof(avs_node) );

    node->data = data;
    node->prev = list->tail;
    node->next = NULL;

    if( list->tail == NULL ) {
        list->head = node;
    } else {
        list->tail->next = node;
        node->prev = list->tail;
    }

    list->tail = node;

    list->size++;

    return list; */

    return avs_list_insert_after( list, NULL, data );
}

/**
 * @brief Prepend the data to the front of the list
 * 
 * @param list 
 * @param data 
 * @return avs_list* 
 */
avs_list* avs_list_prepend( avs_list *list, void *data ) {
    /* avs_node *node = avs_allocate( sizeof(avs_node) );

    node->data = data;
    node->prev = NULL;
    node->next = list->head;

    if( list->head == NULL ) {
        list->tail = node;
    }

    list->head = node; */

    return avs_list_insert_before( list, list->head, data );
}

/**
 * @brief Inserts the given data before the provided node.
 * 
 * @param list 
 * @param node 
 * @param data 
 * @return avs_list* 
 */
avs_list* avs_list_insert_before( avs_list *list, avs_node *before_node, void *data ) {
    avs_node *node = avs_allocate( sizeof(avs_node) );

    node->data = data;
    node->prev = NULL;
    node->next = before_node;

    if( before_node == NULL ) {
        list->head = node;
        list->tail = node;
    } else if( before_node == list->head ) { // are we inserting at the head of a list already populated?
        list->head = node;

        // do we need to update tail as well?
        if( list->tail == NULL ) {
            list->tail = node;
        }

        before_node->prev = node;
    } else {
        node->prev = before_node->prev;
        before_node->prev = node;
        node->prev->next = node;
    }

    list->size++;

    return list;
}

/**
 * @brief Inserts the given data after the provided node.
 * 
 * @param list 
 * @param node 
 * @param data 
 * @return avs_list* 
 */
avs_list* avs_list_insert_after( avs_list *list, avs_node *after_node, void *data ) {
    avs_node *node = avs_allocate( sizeof(avs_node) );

    node->data = data;
    node->prev = NULL;
    node->next = NULL;

    // after_node being NULL means add to end of list
    // OR if after_node is set to the list's tail
    if( after_node == NULL || after_node == list->tail ) {
        //debugf( "part 1" );
        if( list->tail == NULL ) {
           // debugf( "part 2" );
            list->head = node;
        } else {
            //debugf( "part 3" );
            list->tail->next = node;
            node->prev = list->tail;
        }

        list->tail = node;
    } else {
       // debugf( "part 4" );
        node->next = after_node->next;
        node->next->prev = node;
        node->prev = after_node;
        after_node->next = node;
       // printf("..");
    }

    list->size++;
  //  debugf( "part 5" );

    return list;
}

/**
 * @brief Push data to the top of the list
 * 
 * @param list 
 * @param data 
 * @return avs_list* 
 */
avs_list* avs_list_push( avs_list *list, void *data ) {
    return avs_list_prepend( list, data );
}

/**
 * @brief Pop data off the top of the list
 * 
 * @param list 
 * @return avs_node* 
 */
void* avs_list_pop( avs_list *list ) {
    if( list->head == NULL ) {
        return NULL;
    }
    
    avs_node *node = avs_list_remove( list, list->head );
    void *data = node->data;

    avs_free(node);

    return data;
}

/**
 * @brief Returns data located at index position of the list. 0 based.s
 * 
 * @param list 
 * @param index 
 * @return void* 
 */
void *avs_list_at_index_data( avs_list *list, int index ) {
    avs_node *node = avs_list_at_index_node( list, index );

    if( node == NULL ) {
        return NULL;
    }

    return node->data;
}

/**
 * @brief Returnsa the node located at index position of the list. 0 based.
 * 
 * @param list 
 * @param index 
 * @return avs_node* 
 */
avs_node *avs_list_at_index_node( avs_list *list, int index ) {
    if( index > (list->size - 1) ) {
        return NULL;
    } else if( index < 0 ) {
        return NULL;
    } else {
        avs_node *node = list->head;

        for( int i = 0; i < index; i++ ) {
            node = node->next;
        }

        return node;
    }
}

/**
 * @brief Removes an element from the list, does NOT avs_free memory
 * 
 * @param list Pointer to the avs_list
 * @param node Pointer to the node to remove
 * @return avs_list* NULL if failed, otherwise pointer to the avs_list
 */
avs_node *avs_list_remove( avs_list *list, avs_node *node ) {
    /// TODO: check to ensure the node is in the list

    avs_node *new_next_node = node->next;
    
    if( node->prev == NULL ) {
        // Removing the head of list
        list->head = new_next_node;

        // If node is not the only element, then carry through prev's null
        if( new_next_node != NULL ) {
            new_next_node->prev = NULL;
        }
    } else if( node->next == NULL ) {
        // Removing the tail of the list
        list->tail = node->prev;
        list->tail->next = NULL;
    } else {
        // Removing non-head or tail element
        node->next->prev = node->prev;
        node->prev->next = node->next;
    }
    
    list->size--;

    return node;
}

/**
 * @brief Removes an element from the list and avs_frees the memory
 * 
 * @param list Pointer to the avs_list
 * @param node Pointer to the node to remove
 * @return avs_list* NULL if failed, otherwise pointer to the avs_list
 */
avs_list *avs_list_free( avs_list *list, avs_node *node ) {
    if( avs_list_remove(list, node) != NULL ) {
        avs_free(node);
        return list;
    }

    return NULL;
}

/**
 * @brief Calls function 'for_each_callback' for every node in the list
 * 
 * @param list Pointer to the avs list
 * @param for_each_callback Pointer to the function to call.
 */
void avs_list_for_each( avs_list *list, void (*for_each_callback)(avs_node *) ) {
    if( list == NULL ) {
        return;
    }

    avs_node *n = list->head;

    for( int i = 0; i < list->size; i++ ) {
        for_each_callback(n);
        n = n->next;
    }
}

/**
 * @brief Returns a pointer to the node of the list containing the given data, using the comparison function callback
 * 
 * @param list Pointer to the avs list
 * @param data_to_find Pointer to the data to find
 * @param comparison_callback Pointer to the function to do the comparison
 * @return avs_node* Pointer to the node containing the data, otherwise NULL 
 * 
 * Comparison function results:
 *   - number == a is less than b
 *   0 == a equals b
 *   + number = a is greater than b
 */
avs_node* avs_list_find_data( avs_list *list, void *data_to_find, int (*comparison_callback)(void *, void *) ) {
    if( list == NULL ) {
        return NULL;
    }

    avs_node *n = list->head;
    
    for( int i = 0; i < list->size; i++ ) {
        int compare_result = comparison_callback( data_to_find, n->data );
        if( compare_result == 0 ) {
            return n;
        }

        n = n->next;
    }

    return NULL;
}

/**
 * @brief Empties out the given list
 * 
 * @param list pointer to an avs_list
 */
void avs_list_empty( avs_list *list ) {
    while( list->head ) {
        avs_list_free( list, list->head );
    }

    list->size = 0;
}

/**
 * @brief Enqueues data to the "front" of the list
 * 
 * @param list 
 * @param data 
 * @return avs_list* 
 */
avs_list* avs_list_enqueue( avs_list *list, void *data ) {
    if( list == NULL ) {
        return NULL;
    }

    return avs_list_prepend( list, data );
}

/**
 * @brief Dequeues data from the "back" of the list
 * 
 * @param list 
 * @return void* 
 */
void* avs_list_dequeue( avs_list *list ) {
    if( list == NULL ) {
        return NULL;
    }

    avs_node *node = avs_list_remove( list, list->tail );
    void *data = node->data;

    avs_free(node);

    return data;
}