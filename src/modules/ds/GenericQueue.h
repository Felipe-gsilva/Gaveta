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
bool pop_gq(GenericQueue **gq, void *save_to);

GenericQueue *top_gq(GenericQueue **gq);
bool is_gq_empty(GenericQueue **gq);
bool clear_gq(GenericQueue **gq);
bool search_gq(GenericQueue **gq, void *data, bool (*cmp_fn)(void*, void*), GenericQueue **found_node);

// external
bool export_gq_to_disk(GenericQueue **ll, char* path, write_fallback_fn fn);
bool read_gq_from_disk(GenericQueue **ll, char* path, read_fallback_fn fn);
void print_generic_queue(GenericQueue **gq, print_callback_fn printer);

#define print_gq(gq, T) print_generic_queue(gq, _Generic((T){0}, \
    int:    print_int_node,    \
    float:  print_float_node,  \
    char*:  print_string_node, \
    btree_node: print_gq_btree_node, \
    void*: (T){0}, \
    default: print_int_node))

#define export_gq(gq,  path, T) export_gq_to_disk(gq, path, \
    _Generic((T){0}, \
      int:    write_int_node,    \
      float:  write_float_node,  \
      char*:  write_string_node, \
      void*: (T){0}, \
      default: write_int_node ))

#define read_gq(gq, path, T) read_gq_from_disk(gq, path, \
    _Generic((T){0}, \
      int:    read_int_node,    \
      float:  read_float_node,  \
      char*:  read_string_node, \
      void*: (T){0}, \
      default: read_int_node ))

#endif
