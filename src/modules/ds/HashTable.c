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
  (*ht)->bucket_heads =
      darray_create(sizeof(GenericLinkedList), INITIAL_HT_SIZE);
  if (!(*ht)->bucket_heads) {
    g_error(HASH_TABLE_ERROR, "Failed to create hash table");
    return false;
  }
  for (int i = 0; i < INITIAL_HT_SIZE; i++) {
    GenericLinkedList *l = g_alloc(sizeof(GenericLinkedList));
    init_ll(&l, sizeof(bucket));
    darray_push((*ht)->bucket_heads, l);
  }

  return true;
}

bool clear_ht(HashTable **ht) {
  if (!ht || !*ht) {
    g_error(HASH_TABLE_ERROR, "Invalid Inputs");
    return false;
  }

  for (u32 i = 0; i < darray_size((*ht)->bucket_heads); i++) {
    GenericLinkedList *bl = darray_get_pointer((*ht)->bucket_heads, i);
    if (bl) {
      clear_ll(&bl);
      g_dealloc(bl);
    }
  }

  darray_destroy((*ht)->bucket_heads);
  g_dealloc(*ht);
  *ht = NULL;
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

  if (i >= darray_size((*ht)->bucket_heads)) {
    g_error(HASH_TABLE_ERROR, "Hash index out of bounds");
    return false;
  }

  GenericLinkedList *bl = darray_get_pointer((*ht)->bucket_heads, i);

  if (!bl) {
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

  g_debug(HASH_TABLE_STATUS, "Inserted key: %s at index: %u\n", key, i);
  return true;
}

void *lookup_ht(HashTable **ht, void *k, cmp_fn f) {
  if (!ht || !*ht)
    return NULL;

  u32 i = (*ht)->h(k) % darray_size((*ht)->bucket_heads);
  printf("Looking up key at index: %u\n", i);

  GenericLinkedList *ll =
      (GenericLinkedList *)darray_get_pointer((*ht)->bucket_heads, i);

  if (!ll)
    return NULL;

  bucket search_key = {.key = k};
  bucket *found_key = NULL;
  if (!search_ll(&ll, &search_key, f, (void**)&found_key)) {
    g_debug(HASH_TABLE_STATUS, "Key not found in Hash Table %s\n");
    return NULL;
  }

  if (!found_key) {
    g_error(HASH_TABLE_ERROR, "No data found for the given key");
    return NULL;
  }

  return found_key->data;
}

u32 polynomial_rolling_hash_fn(char *key) {
  int n = strlen(key);
  int p = 31, m = 1e9 + 7;
  u32 h_value = 0;
  int p_pow = 1;
  for (int i = 0; i < n; ++i) {
    h_value = (h_value + (key[i] - 'a' + 1) * p_pow) % m;
    p_pow = (p_pow * p) % m;
  }

  return h_value;
}

u32 djb2(char *str) {
  unsigned long hash = 5381;
  int c;

  while ((c = *str++))
    hash = ((hash << 5) + hash) + c;

  return hash;
}

static inline uint32_t murmur_32_scramble(uint32_t k) {
  k *= 0xcc9e2d51;
  k = (k << 15) | (k >> 17);
  k *= 0x1b873593;
  return k;
}

u32 murmur3_32(char *key) {
  // this is a geek for geeks implementation of murmur3 hash function
  u32 h = 0x12345678;
  u32 k;
  size_t len = strlen(key);
  /* Read in groups of 4. */
  for (size_t i = len >> 2; i; i--) {
    // Here is a source of differing results across endiannesses.
    // A swap here has no effects on hash properties though.
    memcpy(&k, key, sizeof(uint32_t));
    key += sizeof(uint32_t);
    h ^= murmur_32_scramble(k);
    h = (h << 13) | (h >> 19);
    h = h * 5 + 0xe6546b64;
  }
  /* Read the rest. */
  k = 0;
  for (size_t i = len & 3; i; i--) {
    k <<= 8;
    k |= key[i - 1];
  }
  // A swap is *not* necessary here because the preceding loop already
  // places the low bytes in the low places according to whatever endianness
  // we use. Swaps only apply when the memory is copied in a chunk.
  h ^= murmur_32_scramble(k);
  /* Finalize. */
  h ^= len;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}
