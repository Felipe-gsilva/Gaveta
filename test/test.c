#include "test.h"
#include <assert.h>
#include "../src/modules/memory/mem.h"

static bool compare_ints(void *v1, void *v2) {
  assert(v1 && v2);
  printf("Comparing %d and %d\n", *(int*)v1, *(int*)v2);
  int *i1 = v1;
  int *i2 = v2;
  return *i2 == *i1;
}

bool test_queue_search(void) {
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
  return true;
}

bool test_memory_allocation(void) {
  // global memory pool allocation test
  void *p1 = g_alloc(1000);
  if (!p1) {
    g_dealloc(p1);
    return false;
  }
  void *p2 = g_alloc(5000);
  if (!p2) {
    g_dealloc(p1);
    return false;
  }
  g_dealloc(p1);
  g_dealloc(p2);
  // local memory pool allocation test
  memory *local_mem = NULL;
  init_memory(local_mem, 2 * MB);
  void *p3 = g_allocate(local_mem, 2000);
  if (!p3) {
    clear_memory(local_mem);
    return false;
  }
  void *p4 = g_allocate(local_mem, 8000);
  if (!p4) {
    g_deallocate(local_mem, p3);
    clear_memory(local_mem);
    return false;
  }
  g_deallocate(local_mem, p3);
  g_deallocate(local_mem, p4);
  clear_memory(local_mem);
  return true;
}

int main(void) {
  init_mem(4 * MB);
  assert(test_queue_search());
  assert(test_memory_allocation());
  return 0;
}
