#include "test_generic_queue.h"
#include "test_mem.h"

int main(void) {
  assert(test_mem());
  assert(test_generic_queue());
  return 0;
}
