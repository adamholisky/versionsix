/**
 * @file node.h
 * @author Adam Holisky (adam.holisky@gmail.com)
 * @brief Node header file
 * @version 0.1
 * @date 2026-03-07
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef AVSUL_NODE_H
#define AVSUL_NODE_H
#ifdef __cplusplus
extern "C" {
#endif

// I don't like forward declaring like this, but it works.
typedef struct avs_node avs_node;

struct avs_node {
    avs_node *next;
    avs_node *prev;
    void *data;
};



#ifdef __cplusplus
}
#endif
#endif