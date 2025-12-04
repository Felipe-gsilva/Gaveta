#ifndef __GENERIC_QUEUE_H
#define __GENERIC_QUEUE_H

#include "../../defines.h"

typedef struct __generic_queue {
  void *data;
  void *next;
  void *last;
  u32 data_size;
} generic_queue;

bool init_gq(generic_queue **gq, u32 data_size);
bool push_gq(generic_queue **gq, void *data);
bool pop_gq(generic_queue **gq, generic_queue *save_to);

generic_queue *top_gq(generic_queue **gq);
bool is_gq_empty(generic_queue **gq);
bool clear_gq(generic_queue **gq);
bool search_gq(generic_queue **gq, void *data, bool (*cmp_fn)(void*, void*), generic_queue **found_node);

typedef void (*print_callback_fn)(void *data);

void print_int_node(void *data);
void print_float_node(void *data);
void print_string_node(void *data);
void print_gq_btree_node(void *data);

#define print_gq(gq, T) print_generic_queue(gq, _Generic((T){0}, \
    int:    print_int_node,    \
    float:  print_float_node,  \
    char*:  print_string_node, \
    btree_node: print_gq_btree_node, \
    default: print_int_node))

void print_generic_queue(generic_queue **gq, print_callback_fn printer);
#endif
