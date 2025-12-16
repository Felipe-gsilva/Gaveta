#include "test_hash_table.h"

bool test_hash_tables() {
  init_mem(MB * 2);
  // integer test
  Map *ht;
  init_ht(&ht, polynomial_rolling_hash_fn);
  put_ht(&ht, );
  
  return true;
}
