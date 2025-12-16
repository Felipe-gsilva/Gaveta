#include "HashTable.h"

bool init_ht(HashTable **ht, hash_fn h) {
  if (*ht) {
    g_error(HASH_TABLE_ERROR, "Hash table already initialized");
    return false;
  }

  (*ht)->h = h;

  (*ht)->buckets = darray_create(sizeof(GenericLinkedList), INITIAL_HT_SIZE);
  if (!(*ht)->buckets) {
    g_error(HASH_TABLE_ERROR, "Failed to create hash table");
    return false;
  }
  // yet to see how to init these heads
  for (int i = 0; i < INITIAL_HT_SIZE; i ++) {
    GenericLinkedList *l = g_alloc(sizeof(GenericLinkedList));
  }

  return true;
}

bool put_ht(HashTable **ht, const char *key, void *data) {
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

  GenericLinkedList *bucket = g_alloc(sizeof(GenericLinkedList));
  if(darray_get((*ht)->buckets, i, bucket) != 0) {
    g_error(HASH_TABLE_ERROR, "Did not find a valid bucket to insert data!");
    return false;
  }
  if (!insert_ll(&bucket, data)) {
    g_error(HASH_TABLE_ERROR, "Could not insert data into Hash Table bucket");
    return false;
  }
  return true;
}

void *lookup_ht(HashTable **ht, void *k, cmp_fn f) {
  if (!ht || !*ht || !(*ht)->buckets) {
    return NULL;
  }

  u32 i = (*ht)->h(k);
  if (i >= darray_size((*ht)->buckets)) {
    g_error(HASH_TABLE_ERROR, "Hash index out of bounds");
    return NULL;
  }

  GenericLinkedList *ll = g_alloc(sizeof(GenericLinkedList));
  darray_get((*ht)->buckets, i, ll);
  if (!ll) {
    g_error(HASH_TABLE_ERROR, "Key not found in hash table");
    return NULL;
  }

  void *node = g_alloc((*ht)->data_size);
  search_ll(&ll, k, f, node);

  if (!node) return NULL;

  return node;
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
