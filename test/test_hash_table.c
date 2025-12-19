#include "test_hash_table.h"
#include "../src/modules/ds/BTree.h"

bool cmp_str_keys(void *a, void *b) {
    bucket *b1 = (bucket *)a;
    bucket *b2 = (bucket *)b;
    return strcmp(b1->key, b2->key) == 0;
}

bool cmp_btree_node(void *a, void *b) {
  assert(a && b);
  btree_node *b1 = (btree_node *) a;
  btree_node *b2 = (btree_node *) b;
  return b1->rrn == b2->rrn;
}

static bool test_homogeneous_ds() {
  printf("--- Starting Hash Table Test ---\n");
  init_mem(MB * 2);

  Map *ht = NULL; 
  if (!init_hash_table(&ht, sizeof(int))) {
      printf("Failed to init HT\n");
      return false;
  }

  int val1 = 100;
  int val2 = 200;
  char *key1 = "username";
  char *key2 = "email";
  put_ht(&ht, key1, &val1);
  put_ht(&ht, key2, &val2);

  int *result = lookup_ht(&ht, key1, cmp_str_keys);
  
  if (!result) {
      printf("TEST FAIL: Key 'username' not found.\n");
      return false;
  }
  
  printf("Found value: %d\n", *result);
  
  if (*result != 100) {
      printf("TEST FAIL: Expected 100, got %d\n", *result);
      return false;
  }

  
  result = lookup_ht(&ht, key2, cmp_str_keys);

  if (!result) {
      printf("TEST FAIL: Key 'email' not found.\n");
      return false;
  }

  if (*result != 200) {
      printf("TEST FAIL: Expected 200, got %d\n", *result);
      return false;
  }

  printf("--- Hash Table Test Passed ---\n");
  
  clear_ht(&ht);
  clear_mem(); 
  return true;
}

bool test_heterogeneous_ds() {
  printf("--- Starting Hash Table Test ---\n");
  init_mem(MB * 2);

  Map *ht = NULL; 
  if (!init_hash_table(&ht, sizeof(btree_node))) {
      printf("Failed to init HT\n");
      return false;
  }
  btree_node g = {.child_num = 0, .children= NULL, .keys = NULL, .leaf = 0, .next_leaf = 0, .rrn = 100, .dirty = true, .pinned = true};

  char *key1 = "username";
  put_ht(&ht, key1, &g);

  btree_node *result = (btree_node*)lookup_ht(&ht, key1, cmp_str_keys);
  
  if (!result) {
    printf("TEST FAIL: Key 'username' not found.\n");
    return false;
  }
  
  printf("Found value: %d\n", result->rrn);
  
  if (!cmp_btree_node(result, &g)) {
      printf("TEST FAIL: Expected 100, got %d\n", result->rrn);
      return false;
  }

  
  printf("--- Hash Table Test Passed ---\n");
  
  clear_ht(&ht);
  clear_mem(); 
  return true;
}

bool test_hash_tables() {
  return (test_homogeneous_ds() && test_heterogeneous_ds());
}
