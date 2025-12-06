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
bool remove_ll(GenericLinkedList **ll, GenericLinkedList *save_to);

bool is_ll_empty(GenericLinkedList **ll);
bool clear_ll(GenericLinkedList **ll);
bool search_ll(GenericLinkedList **ll, void *data, bool (*cmp_fn)(void*, void*), GenericLinkedList **found_node);

typedef void (*print_callback_fn)(void *data);

#define print_ll(ll, T) print_generic_LinkedList(ll, _Generic((T){0}, \
    int:    print_int_node,    \
    float:  print_float_node,  \
    char*:  print_string_node, \
    btree_node: print_ll_btree_node, \
    default: print_int_node))

void print_generic_LinkedList(GenericLinkedList **ll, print_callback_fn printer);
#endif
