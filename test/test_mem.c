#include "test_mem.h"

bool test_mem(void) {
  // global memory pool allocation test
  init_mem(4 * MB);
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
  clear_mem();
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
