#ifndef __GENERIC_QUEUE_H
#define __GENERIC_QUEUE_H

#include "../../defines.h"
#include "./ds_utils.h"

typedef struct __GenericQueue {
  void *data;
  u32 data_size;
  struct __GenericQueue *next;
  struct __GenericQueue *tail;
} GenericQueue;

bool init_gq(GenericQueue **gq, u32 data_size);
bool push_gq(GenericQueue **gq, void *data);
bool pop_gq(GenericQueue **gq, GenericQueue *save_to);

GenericQueue *top_gq(GenericQueue **gq);
bool is_gq_empty(GenericQueue **gq);
bool clear_gq(GenericQueue **gq);
bool search_gq(GenericQueue **gq, void *data, bool (*cmp_fn)(void*, void*), GenericQueue **found_node);

typedef void (*print_callback_fn)(void *data);

#define print_gq(gq, T) print_generic_queue(gq, _Generic((T){0}, \
    int:    print_int_node,    \
    float:  print_float_node,  \
    char*:  print_string_node, \
    btree_node: print_gq_btree_node, \
    default: print_int_node))

void print_generic_queue(GenericQueue **gq, print_callback_fn printer);
#endif
