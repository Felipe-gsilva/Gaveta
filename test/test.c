#include "test.h"
#include <assert.h>
#include "../src/modules/memory/mem.h"

static bool compare_ints(void *v1, void *v2) {
  assert(v1 && v2);
  int *i1 = v1;
  int *i2 = v2;
  return *i2 == *i1;
}

void test_queue_search(void) {
  // homogeneous data type
  GenericQueue *gq = NULL;
  init_gq(&gq, sizeof(int)); 
  int val1 = 6;
  int val2 = 8;
  print_gq(&gq, int);
  push_gq(&gq, &val1);
  push_gq(&gq, &val2);
  GenericQueue *entry = NULL;
  search_gq(&gq, &val1, compare_ints, &entry);
  assert(*(int*)entry->data == 6);
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
}

int main(void) {
  init_mem(4 * MB);
  test_queue_search();
  return 0;
}
