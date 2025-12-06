#include "test_generic_queue.h"

bool test_generic_queue(void) {
  init_mem(4 * MB);
  // homogeneous data type
  GenericQueue *gq = NULL;
  init_gq(&gq, sizeof(int)); 
  int val1 = 6;
  int val2 = 8;
  print_gq(&gq, int);
  push_gq(&gq, &val1);
  push_gq(&gq, &val2);
  print_gq(&gq, int);
  GenericQueue *entry = NULL;
  if (!search_gq(&gq, &val1, compare_ints, &entry)) return false;
  if (*(int*)entry->data != 6) return false;
  pop_gq(&gq, NULL);
  pop_gq(&gq, NULL);

  // heterogeneous data type
  GenericQueue *gq2 = NULL;
  init_gq(&gq2, sizeof(btree_node)); 
  btree_node g = {.child_num = 0, .children= NULL, .keys = NULL, .leaf = 0, .next_leaf = 0, .rrn = 0};
  push_gq(&gq2, &g);
  print_gq(&gq2, btree_node);
  push_gq(&gq2, &g);


  pop_gq(&gq2, NULL);
  print_gq(&gq2, btree_node);
  pop_gq(&gq2, NULL);
  print_gq(&gq2, btree_node);
  clear_gq(&gq2);
  clear_gq(&gq);
  clear_mem();
  return true;
}
