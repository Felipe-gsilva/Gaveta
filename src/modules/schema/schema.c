#include "schema.h"

void schema_fn() {
  HashTable* schema_map;
  init_ht(&schema_map, sizeof(ColumnMetadata), 64, polynomial_rolling_hash_fn);
}
