#ifndef __GENERIC_LINKEDLIST_H
#define __GENERIC_LINKEDLIST_H

#include "../../defines.h"
#include "./ds_utils.h"

typedef struct __GenericLinkedList {
  void *data;
  u32 data_size;
  struct __GenericLinkedList *next;
} GenericLinkedList;

bool init_ll(GenericLinkedList **ll, u32 data_size);
GenericLinkedList *create_ll_node(void *data, u32 data_size);
bool insert_ll(GenericLinkedList **ll, void *data);
bool remove_ll(GenericLinkedList **ll, void *save_to);

bool is_ll_empty(GenericLinkedList **ll);
bool clear_ll(GenericLinkedList **ll);
bool search_ll(GenericLinkedList **ll, void *data, cmp_fn f, void **found_data);
u32 get_ll_size(GenericLinkedList **ll);

// external
bool export_ll_to_disk(GenericLinkedList **ll, char* path, write_fallback_fn fn);
bool read_ll_from_disk(GenericLinkedList **ll, char* path, read_fallback_fn fn);
void print_generic_linkedlist(GenericLinkedList **ll, print_callback_fn printer);

#define print_ll(ll, T) print_generic_linkedlist(ll, \
    _Generic((T){0}, \
      int:    print_int_node,    \
      float:  print_float_node,  \
      char*:  print_string_node, \
      u32: print_u32_node, \
      btree_node: print_gq_btree_node, \
      void*: (T){0}, \
      default: print_int_node))

#define write_ll(ll, path, T) export_ll_to_disk(ll, path, \
    _Generic((T){0}, \
      int:    write_int_node,    \
      u32:    write_u32_node,    \
      float:  write_float_node,  \
      char*:  write_string_node, \
      void*: (T){0}, \
      default: write_int_node ))

#define read_ll(ll, path, T) read_ll_from_disk(ll, path, \
    _Generic((T){0}, \
      int:    read_int_node,    \
      u32:    read_u32_node,    \
      float:  read_float_node,  \
      char*:  read_string_node, \
      void*: (T){0}, \
      default: read_int_node ))

void print_generic_LinkedList(GenericLinkedList **ll, print_callback_fn printer);
#endif
