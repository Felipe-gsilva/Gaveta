#include "test_generic_queue.h"
#include "test_mem.h"
#include "test_btree.h"
#include <assert.h>

int main(void) {
  assert(test_mem());
  assert(test_generic_queue());
  assert(test_btree_insert_and_search());
  return 0;
}
