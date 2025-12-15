#include "HashTable.h"

bool init_ht(HashTable **ht, u32 data_size, hash_fn h) {
  if (*ht) {
    g_error(HASH_TABLE_ERROR, "Hash table already initialized");
    return false;
  }

  assert(data_size > 0);
  (*ht)->data_size = data_size;
  (*ht)->h = h;

  (*ht)->buckets = darray_create(sizeof(GenericLinkedList), INITIAL_HT_SIZE);
  if (!(*ht)->buckets) {
    g_error(HASH_TABLE_ERROR, "Failed to create hash table");
    return false;
  }
  for (int i = 0; i < INITIAL_HT_SIZE; i ++) {
    GenericLinkedList *l = g_alloc(sizeof(GenericLinkedList));
    darray_get((*ht)->buckets, i, l);
    if (!init_ll(&l, sizeof(data_size))) {
      g_error(HASH_TABLE_ERROR, "Could not initialize linkedlist buckets");
      return false;
    }
  }

  return true;
}

bool put_ht(HashTable **ht, const char *key, const void *data) {
  if (!ht || !*ht || !(*ht)->buckets) {
    return false;
  }

  if (!data) {
    g_error(HASH_TABLE_ERROR, "Data to insert is NULL");
    return false;
  }

  u32 i = (*ht)->h(key);
  printf("Inserting key: %s at index: %u\n", key, i);

  if (i >= darray_size((*ht)->buckets)) {
    g_error(HASH_TABLE_ERROR, "Hash index out of bounds");
    return false;
  }
  darray_set((*ht)->buckets, i, data);
  return true;
}

void *lookup_ht(HashTable **ht, const void *k, compare_fallback_fn f) {
  if (!ht || !*ht || !(*ht)->buckets) {
    return NULL;
  }

  u32 i = (*ht)->h(k);
  if (i >= darray_size((*ht)->buckets)) {
    g_error(HASH_TABLE_ERROR, "Hash index out of bounds");
    return NULL;
  }

  void *data = g_alloc(sizeof((*ht)->data_size));
  darray_get((*ht)->buckets, i, data);
  if (!data) {
    g_error(HASH_TABLE_ERROR, "Key not found in hash table");
    return NULL;
  }

  if (f(data, k)) {
    return data;
  }

  return NULL;
}

u32 polynomial_rolling_hash_fn(const char* key) {
  u32 h_value = 0;
  long long p_pow = 1;
  const int p = 31;
  const int m = INITIAL_HT_SIZE * 10;
  for (int i =0; i< strlen(key); i++) {
    h_value = (h_value + (key[i] - 'a' + 1) * p_pow) % m;
    p_pow = (p_pow * p) % m;
  }

  return h_value;
}
