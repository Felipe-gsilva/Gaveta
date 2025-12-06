#include "HashTable.h"

bool init_ht(HashTable **ht, u32 data_size, hash_fn (h)(key)) {
  if (*ht) {
    g_error(HASH_TABLE_ERROR, "Hash table already initialized");
    return false;
  }

  assert(data_size > 0);
  (*ht)->data_size = data_size;
  (*ht)->h = h;

  (*ht)->val = (DynamicArray**)darray_create(sizeof(DynamicArray), INITIAL_HT_SIZE);
  if (!(*ht)->val) {
    g_error(HASH_TABLE_ERROR, "Failed to create hash table");
    return false;
  }
  for (int i = 0; i < INITIAL_HT_SIZE; i++) {
    (*ht)->val[i] = darray_create(data_size, 1);
    if (!(*ht)->val[i]) {
      g_error(HASH_TABLE_ERROR, "Failed to create hash table bucket");
      return false;
    }
  }

  return true;
}

bool insert_ht(HashTable **ht, void *data) {
  if (!ht || !*ht || !(*ht)->val) {
    return false;
  }

  if (!data) {
    g_error(HASH_TABLE_ERROR, "Data to insert is NULL");
    return false;
  }

  int i = *(int*)(*ht)->h(data);
  if (i < 0) {
    g_error(HASH_TABLE_ERROR, "Hash function returned negative index");
    return false;
  }

  if (i >= darray_size((DynamicArray*)(*ht)->val)) {
    darray_push((DynamicArray*)(*ht)->val, NULL);
  }

  if (!darray_push((*ht)->val[i], data)) {
    g_error(HASH_TABLE_ERROR, "Failed to insert data into hash table");
    return false;
  }

  return true;
}
bool lookup_ht(HashTable **ht, key k) {

}
