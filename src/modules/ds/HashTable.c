#include "HashTable.h"

bool init_ht(HashTable **ht, u32 data_size, u16 max_key_size, hash_fn h) {
  *ht = g_alloc(sizeof(HashTable));
  if (!*ht) {
    g_error(HASH_TABLE_ERROR, "Failed to allocate Hash Table struct");
    return false;
  }

  (*ht)->h = h;
  (*ht)->data_size = data_size;
  (*ht)->max_key_size = max_key_size;
  (*ht)->bucket_heads = darray_create(sizeof(GenericLinkedList), INITIAL_HT_SIZE);
  if (!(*ht)->bucket_heads) {
    g_error(HASH_TABLE_ERROR, "Failed to create hash table");
    return false;
  }
  for (int i = 0; i < INITIAL_HT_SIZE; i ++) {
    GenericLinkedList *l = g_alloc(sizeof(GenericLinkedList));
    init_ll(&l, sizeof(bucket));
    darray_push((*ht)->bucket_heads, l);
  }

  return true;
}

bool put_ht(HashTable **ht, char *key, void *data) {
  if (!ht || !*ht || !(*ht)->bucket_heads) {
    g_error(HASH_TABLE_ERROR, "Invalid Inputs");
    return false;
  }

  if (!key) {
    g_error(HASH_TABLE_ERROR, "Key to insert is NULL");
    return false;
  }

  if (!data) {
    g_error(HASH_TABLE_ERROR, "Data to insert is NULL");
    return false;
  }

  u32 i = (*ht)->h(key) % darray_size((*ht)->bucket_heads);
  printf("Inserting key: %s at index: %u\n", key, i);

  if (i >= darray_size((*ht)->bucket_heads)) {
    g_error(HASH_TABLE_ERROR, "Hash index out of bounds");
    return false;
  }

 GenericLinkedList *bl = NULL;
  if(darray_get_pointer((*ht)->bucket_heads, i, (void**)&bl) != 0) {
    g_error(HASH_TABLE_ERROR, "Did not find a valid bucket to insert data!");
    return false;
  }

  bucket *b = g_alloc(sizeof(bucket));

  b->data = g_alloc((*ht)->data_size);
  memcpy(b->data, data, (*ht)->data_size);

  b->key = g_alloc((*ht)->max_key_size + 1);
  snprintf(b->key, (*ht)->max_key_size + 1, "%s", key);

  if (!insert_ll(&bl, b)) {
    g_error(HASH_TABLE_ERROR, "Could not insert data into Hash Table bucket");
    return false;
  }

  return true;
}

void *lookup_ht(HashTable **ht, void *k, cmp_fn f) {
  if (!ht || !*ht) return NULL;

  u32 hash = (*ht)->h(k);
  u32 i = hash % darray_size((*ht)->bucket_heads);

  GenericLinkedList *ll = NULL;
  darray_get_pointer((*ht)->bucket_heads, i, (void**)&ll);

  if (!ll) return NULL;

  bucket search_key = { .key = k };
  bucket *found = g_alloc(sizeof(bucket));
  search_ll(&ll, &search_key, f, (void*)found);

  if (!found) return NULL;
  return found->data;
}

u32 polynomial_rolling_hash_fn(const char* key) {
  u32 h_value = 0;
  long long p_pow = 1;
  const int p = 31;
  const int m = 1e9 + 9;
  for (int i =0; i< strlen(key); i++) {
    h_value = (h_value + (key[i] - 'a' + 1) * p_pow) % m;
    p_pow = (p_pow * p) % m;
  }

  return h_value;
}
