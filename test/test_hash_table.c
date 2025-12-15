#include "test_hash_table.h"

bool test_hash_tables() {
  init_mem(MB * 2);
  // integer test
  Map *ht;
  init_ht(&ht, sizeof(int), modulo);
  
  return true;
}
