#include "test_hash_table.h"

bool cmp_str_keys(void *a, void *b) {
    bucket *b1 = (bucket *)a;
    bucket *b2 = (bucket *)b;
    return strcmp(b1->key, b2->key) == 0;
}

bool test_hash_tables() {
  printf("--- Starting Hash Table Test ---\n");
  init_mem(MB * 2);

  Map *ht = NULL; 
  if (!init_ht(&ht, sizeof(int), 32, polynomial_rolling_hash_fn)) {
      printf("Failed to init HT\n");
      return false;
  }

  int val1 = 100;
  int val2 = 200;
  char *key1 = "username";
  char *key2 = "email";

  printf("Putting 'username' -> 100\n");
  put_ht(&ht, key1, &val1);
  
  printf("Putting 'email' -> 200\n");
  put_ht(&ht, key2, &val2);

  printf("Looking up 'username'...\n");
  // We construct a dummy bucket for the search key if your compare fn expects it
  bucket search_key = {.key = "username"};
  
  int *result = lookup_ht(&ht, &search_key, cmp_str_keys);
  
  if (!result) {
      printf("TEST FAIL: Key 'username' not found.\n");
      return false;
  }
  
  printf("Found value: %d\n", *result);
  
  if (*result != 100) {
      printf("TEST FAIL: Expected 100, got %d\n", *result);
      return false;
  }

  printf("--- Hash Table Test Passed ---\n");
  
  clear_mem(); 
  return true;
}
