#include "test_generic_list.h"

bool test_generic_linkedlist() {
  GenericLinkedList *ll;
  init_ll(&ll, sizeof(int));

  assert(is_ll_empty(&ll) == true);

  int val1 = 10;
  int val2 = 20;
  int val3 = 30;

  insert_ll(&ll, &val1);
  insert_ll(&ll, &val2);
  insert_ll(&ll, &val3);

  assert(is_ll_empty(&ll) == false);

  GenericLinkedList *found_node = NULL;
  int search_val = 20;
  bool found = search_ll(&ll, &search_val, compare_ints, found_node->data);
  assert(found == true);
  assert(*(int*)found_node->data == search_val);

  GenericLinkedList *removed_node = create_ll_node(NULL, sizeof(int));
  remove_ll(&ll, removed_node);
  assert(*(int*)removed_node->data == val3);
  g_dealloc(removed_node->data);
  g_dealloc(removed_node);

  clear_ll(&ll);
  assert(is_ll_empty(&ll) == true);

  g_dealloc(ll);

  printf("Generic LinkedList tests passed!\n");
  return true;
}
