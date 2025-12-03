#include "test.h"

void test_queue_search(void) {
  // homogeneous data type
  generic_queue *gq = NULL;
  init_generic_queue(&gq, sizeof(int)); 
  int val1 = 6;
  int val2 = 8;
  push_generic_queue(&gq, &val1);
  push_generic_queue(&gq, &val2);
  print_gq(&gq, int); 
  pop_generic_queue(&gq, NULL);
  print_gq(&gq, int);
  pop_generic_queue(&gq, NULL);
  print_gq(&gq, int);

  // heterogeneous data type
  generic_queue *gq2 = NULL;
  init_generic_queue(&gq2, sizeof(btree_node)); 
  btree_node g = {.child_num = 0, .children= NULL, .keys = NULL, .leaf = 0, .next_leaf = 0, .rrn = 0};
  push_generic_queue(&gq2, &g);
  print_gq(&gq2, btree_node);
  push_generic_queue(&gq2, &g);
  print_gq(&gq2, btree_node);
  pop_generic_queue(&gq2, NULL);
  print_gq(&gq2, btree_node);
  pop_generic_queue(&gq2, NULL);
  print_gq(&gq2, btree_node);
}

int main(void) {
  test_queue_search();
  return 0;
}
