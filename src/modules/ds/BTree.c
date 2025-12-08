#include "BTree.h"
#include "../memory/mem.h"
#include "../log/log.h"
#include "../config/config.h"

btree_node **g_allocated;
u16 g_n = 0;

BTree *create_btree(const char *config_file) {
  btree_config cfg;

  if (!read_btree_config(config_file, &cfg)) {
    g_error(BTREE_ERROR, "Could not read B-Tree config file");
    return NULL;
  }

  BTree *b = alloc_tree_buf(cfg.order);
  if (!b) {
    g_error(BTREE_ERROR, "Could not create B-Tree");
    return NULL;
  }
  b->config = cfg;

  char *index_file = g_alloc(MAX_ADDRESS), *data_file = g_alloc(MAX_ADDRESS);
  sprintf(index_file, "assets/public/btree%c%s.idx", '-', b->config.name);
  sprintf(data_file, "assets/public/btree%c%s.dat", '-', b->config.name);

  create_index_file(b->idx, index_file);
  create_data_file(b->data, data_file);

  load_file(b->idx, index_file, "index");
  load_file(b->data, data_file, "data");

  g_dealloc(data_file);
  g_dealloc(index_file);

  // TODO not just idx, data also needs to load free_rrn
  load_list(b->free_rrn, b->idx->br->free_rrn_address);

  btree_node *temp = load_btree_node(b, b->idx->br->root_rrn);
  b->root = temp;

  if (ftell(b->idx->fp) <= b->idx->br->header_size) {
    int k = 0;
    insert_ll(&b->free_rrn, &k);
    build_tree(b, b->config.order);

    print_gq(&b->q, btree_node);

    k = b->config.order + 1;
    insert_ll(&b->free_rrn, &k);
    b->idx->br->root_rrn = b->root->rrn;
    write_index_header(b->idx);
  }
  return b;
}

void print_gq_btree_node(void *data) {
  btree_node buffer = *(btree_node*)data;
  printf("%d\n", buffer.rrn);
}


BTree *alloc_tree_buf(u32 order) {
  BTree *b = g_alloc(sizeof(BTree));
  if (!b) {
    g_error(BTREE_ERROR, "Could not allocate b_tree_buf_BUFFER");
    return NULL;
  }

  b->root = NULL;
  b->idx = alloc_io_buf();
  if (!b->idx) {
    g_dealloc(b);
    g_error(BTREE_ERROR, "Could not allocate io_buf");
    return NULL;
  }
  b->data = alloc_io_buf();
  if (!b->data) {
    g_dealloc(b->idx);
    g_dealloc(b);
    g_error(BTREE_ERROR, "Could not allocate io_buf");
    return NULL;
  }

  b->q = NULL;
  init_gq(&b->q, sizeof(btree_node));

  if (!b->q) {
    g_dealloc(b->idx);
    g_dealloc(b->data);
    g_dealloc(b);
    g_error(BTREE_ERROR, "Could not allocate queue");
    return NULL;
  }

  b->free_rrn = NULL;
  init_ll(&b->free_rrn, sizeof(u16));
  if (!b->free_rrn) {
    g_dealloc(b->q);
    g_dealloc(b->idx);
    g_dealloc(b->data);
    g_dealloc(b);
    g_error(BTREE_ERROR, "Could not allocate ilist");
    return NULL;
  }

  g_info("Allocated B_TREE_BUFFER");
  return b;
}

bool compare_btree_nodes(void *v1, void *v2) {
  btree_node *b1 = (btree_node*) v1;
  btree_node *b2 = (btree_node*) v2;
  assert((b1 != NULL) && (b2 != NULL));
  return (b1->rrn == b2->rrn);
}

void clear_btree(BTree *b) {
  if (b) {
    clear_ll(&b->free_rrn);
    clear_gq(&b->q);
    clear_io_buf(b->idx);
    clear_io_buf(b->data);
    if (b->root) {
      btree_node *q_btree_node = g_alloc(sizeof(btree_node));
      GenericQueue *node = g_alloc(sizeof(GenericQueue));
      search_gq(&b->q, &b->root, compare_btree_nodes, &node);
      if (node->data) q_btree_node = *(btree_node **)node->data;
      if (!q_btree_node) {
        b->root = NULL;
      } else {
        clear_btree_node(q_btree_node);
        b->root = NULL;
      }
    }
    g_dealloc(b);
    b = NULL;
  }
  g_debug(BTREE_STATUS, "B_TREE_BUFFER cleared");
}

void populate_index_header(index_header_record *bh, const char *file_name) {
  if (bh == NULL) {
    g_error(BTREE_ERROR, "Header pointer is NULL, cannot populate");
    return;
  }

  bh->btree_node_size = sizeof(btree_node);
  bh->root_rrn = 0;
  strcpy(bh->free_rrn_address, file_name);
  bh->free_rrn_address[strlen(file_name) + 1] = '\0';
  bh->header_size = (sizeof(u16) * 3) + strlen(file_name) + 1;
}

bool build_tree(BTree *b, u32 n) {
  if (!b || !b->data) {
    g_error(BTREE_ERROR, "Invalid parameters");
    return false;
  }
  if (!b->free_rrn) {
    load_list(b->free_rrn, b->idx->br->free_rrn_address);
    if (b->free_rrn) {
      puts("Loaded rrn list");
    }
  }
  data_record *d;
  for (int i = 0; i < n; i++) {
    d = load_data_record(b->data, i);
    if (!d) {
      printf("!!Failed to load record %d\n", i);
      continue;
    }

    btree_status status = b_insert(b, d, i);
    if ((status != BTREE_SUCCESS) && (status != BTREE_INSERTED_IN_BTREE_NODE)) {
      g_crit_error(BTREE_ERROR, "!!Failed to insert record %d, error: %d\n", i - 1, status);
    }
  }
  if (d)
    g_dealloc(d);

  g_info("Built tree");
  return true;
}

btree_node *load_btree_node(BTree *b, u16 rrn) {
  if (!b || !b->idx) {
    g_error(BTREE_ERROR, "Error: invalid parameters");
    return NULL;
  }

  // TODO change because of RRN
  btree_node *bn = g_alloc(sizeof(btree_node));
  bn->rrn = rrn;
  GenericQueue *node = g_alloc(sizeof(GenericQueue));
  search_gq(&b->q, &rrn, compare_btree_nodes, &node);

  bn = *(btree_node **)node->data;

  if (bn) {
    g_debug(BTREE_STATUS, "Btree_node found in queue");
    return bn;
  }

  bn = alloc_btree_node(b->config.order);
  if (!bn)
    return NULL;

  bn->rrn = rrn;

  size_t byte_offset =
      (size_t)(b->idx->br->header_size) + ((size_t)(b->idx->br->btree_node_size) * rrn);

  if (fseek(b->idx->fp, byte_offset, SEEK_SET) != 0) {
    g_dealloc(bn);
    return NULL;
  }

  size_t bytes_read = fread(bn, 1, b->idx->br->btree_node_size, b->idx->fp);
  if (bytes_read != b->idx->br->btree_node_size) {
    g_dealloc(bn);
    return NULL;
  }

  push_gq(&b->q, &bn);

  return bn;
}

int write_root_rrn(BTree *b, u16 rrn) {
  if (!b) {
    g_error(BTREE_ERROR, "Error: NULL BTree");
    return BTREE_ERROR_IO;
  }

  b->idx->br->root_rrn = rrn;

  fseek(b->idx->fp, 0, SEEK_SET);
  size_t flag = fwrite(&rrn, sizeof(u16), 1, b->idx->fp);
  if (flag != 1) {
    g_error(BTREE_ERROR, "Error: Could not update root rrn");
    exit(-1);
  }

  fflush(b->idx->fp);

  return BTREE_SUCCESS;
}

void b_update(BTree *b, GenericLinkedList *free_rrn,
              const char *placa) {
 // TODO

}

btree_node *b_search(BTree *b, const char *query_key, u16 *return_pos) {
  if (!b || !b->root || !query_key)
    return NULL;

  key k;
  sprintf((char *)k.id, "%s", query_key);

  btree_node *found_btree_node = NULL;
  *return_pos = search_key(b, b->root, k, return_pos, &found_btree_node);

  if (found_btree_node->leaf)
    return found_btree_node;

  *return_pos = (u16)-1;
  return NULL;
}

void b_range_search(BTree *b, key_range *range) {
  if (!b || !range || !b->root) {
    g_error(BTREE_ERROR, "Invalid parameters for range search");
    return;
  }

  btree_node *curr = b->root;
  while (!curr->leaf) {
    int i;
    for (i = 0; i < curr->keys_num; i++) {
      if (strcmp(range->start_id->id, curr->keys[i].id) < 0) {
        break;
      }
    }
    curr = load_btree_node(b, curr->children[i]);
    if (!curr) {
      g_error(BTREE_ERROR, "Error loading btree_node during range search");
      return;
    }
  }

  bool found_any = false;
  while (curr) {
    if (curr->keys_num > 0 && strcmp(curr->keys[0].id, range->end_id->id) > 0) {
      break;
    }

    for (int i = 0; i < curr->keys_num; i++) {
      if (strcmp(curr->keys[i].id, range->end_id->id) > 0) {
        break;
      }

      if (strcmp(curr->keys[i].id, range->start_id->id) >= 0) {
        found_any = true;
        data_record *record =
            load_data_record(b->data, curr->keys[i].data_register_rrn);
        if (record) {
          // TODO not used print_data_record, since no schema yet exists
          // print_data_record(record); 
          g_dealloc(record);
        }
      }
    }

    if (curr->next_leaf == (u16)-1) {
      break;
    }

    btree_node *next = load_btree_node(b, curr->next_leaf);
    curr = next;
  }

  if (!found_any) {
    puts("Nenhum registro encontrado no intervalo especificado.");
  }
}

int search_in_btree_node(btree_node *p, key key, int *return_pos) {
  if (!p) {
    g_error(BTREE_ERROR, "Error: no btree_node");
    return BTREE_ERROR_INVALID_btree_node;
  }

  for (int i = 0; i < p->keys_num; i++) {
    if (((char*)p->keys[i].id)[0] == '\0') {
      *return_pos = i;
      return BTREE_NOT_FOUND_KEY;
    }

    if (strcmp(p->keys[i].id, key.id) == 0) {
      puts("Curr key was found");
      *return_pos = i;
      return BTREE_FOUND_KEY;
    }

    if (strcmp(p->keys[i].id, key.id) > 0) {
      *return_pos = i;
      return BTREE_NOT_FOUND_KEY;
    }
  }

  *return_pos = p->keys_num;
  return BTREE_NOT_FOUND_KEY;
}

u16 search_key(BTree *b, btree_node *p, key k, u16 *found_pos,
               btree_node **return_btree_node) {
  if (!p)
    return (u16)-1;

  int pos;
  int result = search_in_btree_node(p, k, &pos);

  g_debug(BTREE_STATUS, "btree_node key id: %s     key id: %s\n",
          p->keys_num > 0 ? p->keys[0].id : "", k.id);

  if (result == BTREE_FOUND_KEY) {
    *found_pos = pos;
    *return_btree_node = p;
    return pos;
  }

  if (p->leaf) {
    *return_btree_node = p;
    *found_pos = pos;
    return (u16)-1;
  }

  btree_node *next = load_btree_node(b, p->children[pos]);
  if (!next)
    return (u16)-1;

  u16 ret = search_key(b, next, k, found_pos, return_btree_node);

  if (next != b->root && !search_gq(&b->q, &next, compare_btree_nodes, NULL)) {
    g_dealloc(next);
  }

  return ret;
}

void populate_key(key *k, data_record *d, u16 rrn) {
  if (!k || !d)
    return;

  strncpy(k->id, d->key, 0); // TODO key size minus null terminator
  k->data_register_rrn = rrn;

  g_debug(BTREE_STATUS, "Populated key with ID: %s and data RRN: %hu\n", k->id,
          k->data_register_rrn);
}

btree_status insert_in_btree_node(btree_node *p, key k, btree_node *r_child, int pos) {
  if (!p)
    return BTREE_ERROR_INVALID_btree_node;

  for (int i = p->keys_num - 1; i >= pos; i--) {
    p->keys[i + 1] = p->keys[i];
  }

  p->keys[pos] = k;
  p->keys_num++;

  if (!p->leaf && r_child) {
    for (int i = p->child_num - 1; i >= pos + 1; i--) {
      p->children[i + 1] = p->children[i];
    }
    p->children[pos + 1] = r_child->rrn;
    p->child_num++;
  }

  g_info("After insertion - keys: %d, children: %d\n", p->keys_num,
         p->child_num);
  g_info("Inserted key with data RRN: %hu\n", k.data_register_rrn);

  return BTREE_INSERTED_IN_BTREE_NODE;
}

btree_status b_insert(BTree *b, void *d, u16 rrn) {
  if (!b || !b->data || !d)
    return BTREE_ERROR_INVALID_btree_node;

  key new_key;
  populate_key(&new_key, d, rrn);

  if (!b->root) {
    b->root = alloc_btree_node(b->config.order);
    if (!b->root)
      return BTREE_ERROR_MEMORY;

    b->root->rrn = get_free_rrn(b->free_rrn);
    if (b->root->rrn < 0) {
      g_dealloc(b->root);
      b->root = NULL;
      return BTREE_ERROR_IO;
    }

    b->root->keys[0] = new_key;
    b->root->keys_num = 1;
    b->root->leaf = true;

    return write_index_record(b, b->root);
  }

  key promo_key;
  btree_node *r_child = NULL;
  bool promoted = false;

  btree_status status =
      insert_key(b, b->root, new_key, &promo_key, &r_child, &promoted);

  if (status < 0) {
    return status;
  }

  if (promoted) {
    btree_node *new_root = alloc_btree_node(b->config.order);
    if (!new_root)
      return BTREE_ERROR_MEMORY;

    new_root->rrn = get_free_rrn(b->free_rrn);
    if (new_root->rrn < 0) {
      g_dealloc(new_root);
      return BTREE_ERROR_IO;
    }

    new_root->leaf = false;
    new_root->keys[0] = promo_key;
    new_root->keys_num = 1;
    new_root->children[0] = b->root->rrn;
    new_root->children[1] = r_child->rrn;
    new_root->child_num = 2;

    b->root = new_root;

    btree_status write_status = write_root_rrn(b, b->root->rrn);
    if (write_status < 0)
      return write_status;

    write_status = write_index_record(b, b->root);
    if (write_status < 0)
      return write_status;
  }

  return BTREE_SUCCESS;
}

btree_status b_split(BTree *b, btree_node *p, btree_node **r_child, key *promo_key,
                     key *incoming_key, bool *promoted) {
  if (!b || !p || !r_child || !promo_key || !incoming_key)
    return BTREE_ERROR_INVALID_btree_node;

  key temp_keys[b->config.order];
  u16 temp_children[b->config.order + 1];

  memset(temp_keys, 0, sizeof(temp_keys));
  memset(temp_children, 0xFF, sizeof(temp_children));

  for (int i = 0; i < p->keys_num; i++) {
    temp_keys[i] = p->keys[i];
  }

  if (!p->leaf) {
    for (int i = 0; i <= p->child_num; i++) {
      temp_children[i] = p->children[i];
    }
  }

  int pos = p->keys_num - 1;
  while (pos >= 0 && strcmp(temp_keys[pos].id, incoming_key->id) > 0) {
    temp_keys[pos + 1] = temp_keys[pos];
    if (!p->leaf) {
      temp_children[pos + 2] = temp_children[pos + 1];
    }
    pos--;
  }

  temp_keys[pos + 1] = *incoming_key;
  if (!p->leaf && *r_child) {
    temp_children[pos + 2] = (*r_child)->rrn;
  }

  btree_node *new_btree_node = alloc_btree_node(b->config.order);
  if (!new_btree_node)
    return BTREE_ERROR_MEMORY;

  new_btree_node->rrn = get_free_rrn(b->free_rrn);
  if (new_btree_node->rrn == (u16)-1) {
    g_dealloc(new_btree_node);
    return BTREE_ERROR_IO;
  }

  int split = (b->config.order - 1) / 2;

  if (p->leaf) {
    p->keys_num = split + 1;
    new_btree_node->keys_num = b->config.order - (split + 1);
    new_btree_node->leaf = true;

    for (int i = 0; i < p->keys_num; i++) {
      p->keys[i] = temp_keys[i];
    }

    for (int i = 0; i < new_btree_node->keys_num; i++) {
      new_btree_node->keys[i] = temp_keys[i + split + 1];
    }

    new_btree_node->next_leaf = p->next_leaf;
    p->next_leaf = new_btree_node->rrn;

    *promo_key = new_btree_node->keys[0];
  } else {
    p->keys_num = split;
    new_btree_node->keys_num = b->config.order - split - 1;
    new_btree_node->leaf = false;

    for (int i = 0; i < p->keys_num; i++) {
      p->keys[i] = temp_keys[i];
    }

    *promo_key = temp_keys[split];

    for (int i = 0; i < new_btree_node->keys_num; i++) {
      new_btree_node->keys[i] = temp_keys[i + split + 1];
    }

    for (int i = 0; i <= p->keys_num; i++) {
      p->children[i] = temp_children[i];
    }

    for (int i = 0; i <= new_btree_node->keys_num; i++) {
      new_btree_node->children[i] = temp_children[i + split + 1];
    }

    p->child_num = p->keys_num + 1;
    new_btree_node->child_num = new_btree_node->keys_num + 1;
  }

  btree_status status;
  if ((status = write_index_record(b, p)) != BTREE_SUCCESS) {
    g_dealloc(new_btree_node);
    return status;
  }

  if ((status = write_index_record(b, new_btree_node)) != BTREE_SUCCESS) {
    g_dealloc(new_btree_node);
    return status;
  }

  *r_child = new_btree_node;
  *promoted = true;

  return BTREE_PROMOTION;
}
btree_status insert_key(BTree *b, btree_node *p, key k, key *promo_key,
                        btree_node **r_child, bool *promoted) {
  if (!b || !promo_key || !p)
    return BTREE_ERROR_INVALID_btree_node;

  int pos;
  btree_status status = search_in_btree_node(p, k, &pos);
  if (status == BTREE_FOUND_KEY)
    return BTREE_ERROR_DUPLICATE;

  if (!p->leaf) {
    btree_node *child = load_btree_node(b, p->children[pos]);
    if (!child)
      return BTREE_ERROR_IO;

    key temp_key;
    btree_node *temp_child = NULL;
    status = insert_key(b, child, k, &temp_key, &temp_child, promoted);

    if (child != b->root && !search_gq(&b->q, &child, compare_btree_nodes, NULL)) {
      clear_btree_node(child);
    }

    if (status == BTREE_PROMOTION) {
      k = temp_key;
      *r_child = temp_child;
      if (p->keys_num < b->config.order - 1) {
        *promoted = false;
        status = insert_in_btree_node(p, k, temp_child, pos);
        if (status == BTREE_INSERTED_IN_BTREE_NODE) {
          return write_index_record(b, p);
        }
        return status;
      }
      return b_split(b, p, r_child, promo_key, &k, promoted);
    }
    return status;
  }

  if (p->keys_num < b->config.order - 1) {
    *promoted = false;
    status = insert_in_btree_node(p, k, NULL, pos);
    if (status == BTREE_INSERTED_IN_BTREE_NODE) {
      return write_index_record(b, p);
    }
    return status;
  }

  return b_split(b, p, r_child, promo_key, &k, promoted);
}

btree_status b_remove(BTree *b, char *key_id) {
  if (!b || !b->root || !b->data || !key_id)
    return BTREE_ERROR_INVALID_btree_node;

  u16 pos;
  btree_node *p = b_search(b, key_id, &pos);
  if (!p || strcmp(p->keys[pos].id, key_id) != 0) {
      puts("Key not found");
    return BTREE_NOT_FOUND_KEY;
  }

  if (p->leaf) {
    g_info("Removing key from leaf btree_node RRN: %hu at position: %hu\n", p->rrn,
           pos);
    u16 data_rrn = p->keys[pos].data_register_rrn;

    for (int i = pos; i < p->keys_num - 1; i++)
      p->keys[i] = p->keys[i + 1];
    p->keys_num--;

    if (data_rrn != (u16)-1) {
      if (fseek(b->data->fp,
                b->data->hr->header_size + (data_rrn * sizeof(data_record)),
                SEEK_SET) == 0) {
        data_record empty_record;
        memset(&empty_record, '*', sizeof(data_record));
        fwrite(&empty_record, sizeof(data_record), 1, b->data->fp);
        fflush(b->data->fp);
        insert_ll(&b->free_rrn, &data_rrn);
      }
    }

    if (p == b->root && p->keys_num == 0) {
      clear_btree_node(b->root);
      b->root = NULL;
      return BTREE_SUCCESS;
    }

    btree_status status = write_index_record(b, p);
    if (status < 0)
      return status;

    if (p != b->root && p->keys_num < (b->config.order - 1) / 2) {
      g_info("Leaf underflow detected");

      btree_node *left = get_sibling(b, p, true);
      if (left && left->keys_num > (b->config.order - 1) / 2) {
        return redistribute(b, left, p, true);
      }

      btree_node *right = get_sibling(b, p, false);
      if (right && right->keys_num > (b->config.order - 1) / 2) {
        return redistribute(b, right, p, false);
      }

      if (left) {
        return merge(b, left, p);
      } else if (right) {
        return merge(b, p, right);
      }
    }

    return BTREE_SUCCESS;
  }

  g_info("Key found in internal node - not removing");
  return BTREE_SUCCESS;
}

btree_status redistribute(BTree *b, btree_node *donor, btree_node *receiver,
                          bool from_left) {
  if (!b || !donor || !receiver)
    return BTREE_ERROR_INVALID_btree_node;

  if (from_left) {
    for (int i = receiver->keys_num; i > 0; i--)
      receiver->keys[i] = receiver->keys[i - 1];
    receiver->keys[0] = donor->keys[donor->keys_num - 1];
    donor->keys_num--;
    receiver->keys_num++;
  } else {
    receiver->keys[receiver->keys_num] = donor->keys[0];
    receiver->keys_num++;

    for (int i = 0; i < donor->keys_num - 1; i++) {
      donor->keys[i] = donor->keys[i + 1];
    }
    donor->keys_num--;
  }

  btree_status status = write_index_record(b, donor);
  if (status < 0)
    return status;

  return write_index_record(b, receiver);
}

btree_status merge(BTree *b, btree_node *left, btree_node *right) {
  if (!b || !left || !right)
    return BTREE_ERROR_INVALID_btree_node;

  for (int i = 0; i < right->keys_num; i++) {
    left->keys[left->keys_num + i] = right->keys[i];
  }
  left->keys_num += right->keys_num;

  left->next_leaf = right->next_leaf;

  btree_status status = write_index_record(b, left);
  if (status < 0)
    return status;

  insert_ll(&b->free_rrn, &right->rrn);

  return BTREE_SUCCESS;
}

btree_node *get_sibling(BTree *b, btree_node *p, bool left) {
  if (!b || !p || !b->root)
    return NULL;

  btree_node *parent = find_parent(b, b->root, p);
  if (!parent)
    return NULL;

  int pos;
  for (pos = 0; pos <= parent->child_num; pos++) {
    if (parent->children[pos] == p->rrn)
      break;
  }

  if (left && pos > 0) {
    return load_btree_node(b, parent->children[pos - 1]);
  } else if (!left && pos < parent->child_num - 1) {
    return load_btree_node(b, parent->children[pos + 1]);
  }

  return NULL;
}

void print_btree_node(btree_node *p) {
  if (!p) {
    g_error(BTREE_ERROR, "Página nula");
    return;
  }

  printf("RRN: %hu | Folha: %d | Chaves: %d | Filhos: %d\n", p->rrn, p->leaf,
         p->keys_num, p->child_num);

  printf("Chaves: ");
  for (int i = 0; i < p->keys_num; i++) {
    printf("[%s]", (char*)p->keys[i].id);
  }
  printf("\n");

  if (!p->leaf) {
    printf("RRNs filhos: ");
    for (int i = 0; i < p->child_num; i++) {
      printf("%hu ", p->children[i]);
    }
    printf("\n");
  }

  if (p->leaf) {
    printf("Próxima folha: %hu\n", p->next_leaf);
  }
}

int write_index_header(io_buf *io) {
  if (!io || !io->fp) {
    g_error(BTREE_ERROR, "NULL file");
    return BTREE_ERROR_IO;
  }
  if (io->br == NULL) {
    io->br = g_alloc(sizeof(index_header_record));
    if (io->br == NULL) {
      g_error(BTREE_ERROR, "Memory allocation error");
      return BTREE_ERROR_MEMORY;
    }
  }
  if (io->br->btree_node_size == 0) {
    g_error(BTREE_ERROR, "Error: btree_node size == 0");
    return BTREE_ERROR_INVALID_btree_node;
  }

  size_t free_rrn_len = strlen(io->br->free_rrn_address) + 1;
  io->br->header_size = sizeof(u16) * 3 + free_rrn_len;

  fseek(io->fp, 0, SEEK_SET);

  if (fwrite(&io->br->root_rrn, sizeof(u16), 1, io->fp) != 1) {
    g_error(BTREE_ERROR, "Error while writing root_rrn");
    return BTREE_ERROR_IO;
  }

  if (fwrite(&io->br->btree_node_size, sizeof(u16), 1, io->fp) != 1) {
    g_error(BTREE_ERROR, "Error while writing btree_node_size");
    return BTREE_ERROR_IO;
  }

  if (fwrite(&io->br->header_size, sizeof(u16), 1, io->fp) != 1) {
    g_error(BTREE_ERROR, "Error while writing size");
    return BTREE_ERROR_IO;
  }

  if (fwrite(io->br->free_rrn_address, free_rrn_len, 1, io->fp) != 1) {
    g_error(BTREE_ERROR, "Error while writing free_rrn_address");
    return BTREE_ERROR_IO;
  }

  g_info("Successfully written on index: root_rrn: %hu, btree_node_size: %hu, "
         "size: %hu, "
         "free_rrn_address: %s\n",
         io->br->root_rrn, io->br->btree_node_size, io->br->header_size,
         io->br->free_rrn_address);

  fflush(io->fp);
  return BTREE_SUCCESS;
}

void load_index_header(io_buf *io) {
  if (!io || !io->fp) {
    g_error(BTREE_ERROR, "Invalid IO buffer or file pointer");
    return;
  }

  if (!io->br) {
    io->br = g_alloc(sizeof(index_header_record));
    if (!io->br) {
      g_error(BTREE_ERROR, "Memory allocation error");
      return;
    }
    memset(io->br, 0, sizeof(index_header_record));
  }

  fseek(io->fp, 0, SEEK_SET);

  if (fread(&io->br->root_rrn, sizeof(u16), 1, io->fp) != 1) {
    g_error(BTREE_ERROR, "Error reading root_rrn");
    return;
  }

  if (fread(&io->br->btree_node_size, sizeof(u16), 1, io->fp) != 1) {
    g_error(BTREE_ERROR, "Error reading btree_node_size");
    return;
  }

  if (fread(&io->br->header_size, sizeof(u16), 1, io->fp) != 1) {
    g_error(BTREE_ERROR, "Error reading size");
    return;
  }

  g_info("Loaded index header: root_rrn: %hu\tbtree_node_size: %hu\tsize: %hu\n", io->br->root_rrn,
         io->br->btree_node_size, io->br->header_size);

  size_t rrn_len = io->br->header_size - (3 * sizeof(u16));

  io->br->free_rrn_address = g_alloc(rrn_len + 1);
  if (!io->br->free_rrn_address) {
    g_error(BTREE_ERROR, "Memory allocation error for free_rrn_address");
    return;
  }

  if (fread(io->br->free_rrn_address, rrn_len, 1, io->fp) != 1) {
    g_error(BTREE_ERROR, "Error reading free_rrn_address");
    g_dealloc(io->br->free_rrn_address);
    io->br->free_rrn_address = NULL;
    return;
  }
  io->br->free_rrn_address[rrn_len] = '\0';

  g_info("Index Header Record Loaded\n-->index_header: root_rrn: %hu btree_node_size: %hu size: %hu "
         "free_rrn_list: %s\n",
         io->br->root_rrn, io->br->btree_node_size, io->br->header_size,
         io->br->free_rrn_address);
}

btree_status write_index_record(BTree *b, btree_node *p) {
  if (!b || !b->idx || !p)
    return BTREE_ERROR_IO;

  int byte_offset = b->idx->br->header_size + (b->idx->br->btree_node_size * p->rrn);

  if (fseek(b->idx->fp, byte_offset, SEEK_SET)) {
    g_error(BTREE_ERROR, "Error: could not fseek");
    return BTREE_ERROR_IO;
  }

  size_t written = fwrite(p, b->idx->br->btree_node_size, 1, b->idx->fp);
  if (written != 1) {
    g_error(BTREE_ERROR, "Error: could not write btree_node");
    return BTREE_ERROR_IO;
  }

  fflush(b->idx->fp);

  g_info("Successfully wrote btree_node %hu at offset %d\n", p->rrn, byte_offset);

  if (!search_gq(&b->q, &p, compare_btree_nodes, NULL)) {
    push_gq(&b->q, &p);
  }

  return BTREE_SUCCESS;
}

void create_index_file(io_buf *io, const char *file_name) {
  if (!io || !file_name) {
    g_error(BTREE_ERROR, "Invalid io buffer or file name");
    return;
  }

  strcpy(io->address, file_name);

  if (io->hr == NULL) {
    io->hr = g_alloc(sizeof(data_header_record));
    if (io->hr == NULL) {
      g_error(BTREE_ERROR, "Memory allocation failed for data_header_record");
      return;
    }
  }

  if (io->br == NULL) {
    g_dealloc(io->hr);
    io->hr = NULL;
    io->br = g_alloc(sizeof(index_header_record));
    if (io->br == NULL) {
      g_error(BTREE_ERROR, "Memory allocation failed for index_header_record");
      return;
    }
    io->br->free_rrn_address = g_alloc(sizeof(char) * MAX_ADDRESS);
    if (io->br->free_rrn_address == NULL) {
      g_error(BTREE_ERROR, "Memory allocation failed for free_rrn_address");
      return;
    }
  }

  if (io->fp != NULL) {
    g_error(BTREE_ERROR, "File already opened");
    return;
  }

  g_debug(DISK_STATUS, "Loading file: %s\n", io->address);
  io->fp = fopen(io->address, "r+b");
  if (!io->fp) {
    g_error(DISK_ERROR, "Error opening file: %s. Creating it...\n", io->address);
    io->fp = fopen(io->address, "wb");
    if (io->fp) {
      fclose(io->fp);
    }
    io->fp = fopen(io->address, "r+b");
    if (!io->fp) {
      g_error(BTREE_ERROR, "Failed to create and open file");
      return;
    }
  }

  char list_name[MAX_ADDRESS];
  strcpy(list_name, file_name);
  char *dot = strrchr(list_name, '.');
  if (dot) {
    strcpy(dot, ".hlp");
  }

  load_index_header(io);

  if (strcmp(io->br->free_rrn_address, list_name) != 0) {
    strcpy(io->br->free_rrn_address, list_name);
    populate_index_header(io->br, list_name);
    write_index_header(io);
  }

  g_info("Index file created successfully");
}

btree_node *alloc_btree_node(u32 order) {
  btree_node *p = g_alloc(sizeof(btree_node));

  if (!p) {
    g_error(BTREE_ERROR, "Falha na alocação do nó da btree");
    return NULL;
  }

  memset(p, 0, sizeof(btree_node));
  p->leaf = true;
  p->next_leaf = (u16)-1;

  for (int i = 0; i < order; i++) {
    p->children[i] = (u16)-1;
  }

  return p;
}

bool clear_all_btree_nodes(void) {
  if (!g_allocated) return false;

  for (int i = 0; i < g_n; i++) {
    if (g_allocated[i]) {
      g_dealloc(g_allocated[i]);
      g_allocated[i] = NULL;
    }
  }

  g_dealloc(g_allocated);
  g_allocated = NULL;
  g_n = 0;
  return true;
}

bool clear_btree_node(btree_node *btree_node) {
  if (btree_node) {
    g_dealloc(btree_node);
    return true;
  }

  return false;
}

void track_btree_node(btree_node *p) {
  if (!p)
    return;

  if (!g_allocated) {
    g_allocated = g_alloc(sizeof(btree_node *));
    if (!g_allocated)
      return;
    g_n = 0;
  } else {
    btree_node **tmp = realloc(g_allocated, sizeof(btree_node *) * (g_n + 1));
    if (!tmp)
      return;
    g_allocated = tmp;
  }

  g_allocated[g_n++] = p;
}

btree_node *find_parent(BTree *b, btree_node *current, btree_node *target) {
  if (!b || !current || !target)
    return NULL;

  if (target == b->root)
    return NULL;

  for (int i = 0; i < current->child_num; i++) {
    if (current->children[i] == target->rrn) {
      return current;
    }
  }

  if (!current->leaf) {
    for (int i = 0; i < current->child_num; i++) {
      btree_node *child = load_btree_node(b, current->children[i]);
      if (!child)
        continue;

      btree_node *result = find_parent(b, child, target);

      if (child != b->root && !search_gq(&b->q, &child, compare_btree_nodes, NULL)) {
        clear_btree_node(child);
      }

      if (result)
        return result;
    }
  }

  return NULL;
}


void sort_list(u16 A[], int n) {
  if (n < 1)
    return;
  int h = 1;
  while (h < n / 3) {
    h = 3 * h + 1;
  }

  while (h >= 1) {
    for (int i = h; i < n; i++) {
      int aux = A[i];
      int j = i;
      while (j >= h && A[j - h] > aux) {
        A[j] = A[j - h];
        j -= h;
      }
      A[j] = aux;
    }
    h = h / 3;
  }
}

void write_rrn_list_to_file(GenericLinkedList *i, char *file_name) {
  if (!i )
    return;

  if (!is_ll_empty(&i)) {
    g_error(BTREE_ERROR, "Error: RRN list is not empty");
    return;
  }

  export_ll(&i, file_name, int);
}

// TODO update this to write into header buffer
bool load_list(GenericLinkedList *i, char *s) {
  if (!i || !s) {
    g_error(BTREE_ERROR, "Error: Invalid parameters");
    return false;
  }

  if (!i) read_ll(&i, s, u32);

  if (is_ll_empty(&i)) {
    g_error(BTREE_ERROR, "Error: RRN list is empty");
    return false;
  }


  g_info("Loaded RRN list!");
  return true;
}

u16 get_free_rrn(GenericLinkedList *i) {
  if (!i) {
    g_error(BTREE_ERROR, "Error: Invalid list or file pointer");
    exit(1);
  }


  int rrn;
  remove_ll(&i, &rrn);

  print_ll(&i, u32);
  return rrn;
}

io_buf *alloc_io_buf(void) {
  io_buf *io = g_alloc(sizeof(io_buf));
  if (!io) {
    g_error(BTREE_ERROR, "Could not allocate IO_BUFFER");
    return NULL;
  }

  io->hr = g_alloc(sizeof(data_header_record));
  io->br = g_alloc(sizeof(index_header_record));
  if (!io->hr || !io->br) {
    g_error(BTREE_ERROR, "Could not allocate header records");
    g_dealloc(io);
    return NULL;
  }

  io->hr->free_rrn_address = g_alloc(sizeof(char) * MAX_ADDRESS);
  io->br->free_rrn_address = g_alloc(sizeof(char) * MAX_ADDRESS);
  if (!io->hr->free_rrn_address || !io->br->free_rrn_address) {
    g_error(BTREE_ERROR, "Could not allocate free_rrn_address");
    g_dealloc(io->hr);
    g_dealloc(io->br);
    g_dealloc(io);
    return NULL;
  }

  g_info("Allocated IO_BUFFER");
  return io;
}

void d_insert(io_buf *io, void *d, GenericLinkedList *ld, u16 rrn) {
  if (!io || !d || !ld) {
    g_error(BTREE_ERROR, "Error: NULL parameters on d_insert");
  }
  if (rrn == (u16)-1)
    rrn = get_free_rrn(ld);

  write_data_record(io, d, rrn);
}

void load_data_header(io_buf *io) {
  if (!io || !io->fp) {
    g_error(BTREE_ERROR, "Invalid IO buffer or file pointer");
    return;
  }

  data_header_record temp_hr = {0};

  fseek(io->fp, 0, SEEK_SET);
  if (fread(&temp_hr.header_size, sizeof(u16), 1, io->fp) != 1 ||
      fread(&temp_hr.record_size, sizeof(u16), 1, io->fp) != 1) {
    g_error(BTREE_ERROR, "Error while reading header record (fixed part)");
    return;
  }

  g_debug(BTREE_STATUS, "Header_size: %hu \t Record size: %hu\n", temp_hr.header_size,
          temp_hr.record_size);

  if (!io->hr) {
    io->hr = g_alloc(sizeof(data_header_record));
    if (!io->hr) {
      g_error(BTREE_ERROR, "Memory allocation error");
      return;
    }
  }

  if (temp_hr.header_size > MAX_ADDRESS || temp_hr.header_size < 4 ||
      temp_hr.record_size != sizeof(data_record)) {
    return;
  }

  io->hr->record_size = temp_hr.record_size;
  io->hr->header_size = temp_hr.header_size;

  size_t rrn_len = io->hr->header_size - (2 * sizeof(u16));

  io->hr->free_rrn_address = g_alloc(rrn_len + 1);
  if (!io->hr->free_rrn_address) {
    g_error(BTREE_ERROR, "Memory allocation error for free_rrn_address");
    return;
  }

  fseek(io->fp, sizeof(u16) * 2, SEEK_SET);
  if (fread(io->hr->free_rrn_address, rrn_len, 1, io->fp) != 1) {
    g_error(BTREE_ERROR, "Error while reading free_rrn_address");
    g_dealloc(io->hr->free_rrn_address);
    io->hr->free_rrn_address = NULL;
    return;
  }

  io->hr->free_rrn_address[rrn_len] = '\0';

  g_debug(BTREE_STATUS, "--> data_header: record_size: %hu size: %hu free_rrn_address: %s\n",
          io->hr->record_size, io->hr->header_size, io->hr->free_rrn_address);
}

void *load_data_record(io_buf *io, u16 rrn) {
  if (!io || !io->fp) {
    g_error(BTREE_ERROR, "Invalid IO buffer or file pointer");
    return NULL;
  }

  data_record *hr = g_alloc(sizeof(data_record));

  if (!hr) {
    g_error(BTREE_ERROR, "Memory allocation failed for data record");
    return NULL;
  }

  int byte_offset = io->hr->header_size + (io->hr->record_size * rrn);
  g_debug(BTREE_STATUS, "Header size: %d, Record size: %d, RRN: %d, byte_offset: %d\n",
         io->hr->header_size, io->hr->record_size, rrn, byte_offset);

  if (fseek(io->fp, byte_offset, SEEK_SET) != 0) {
    g_error(BTREE_ERROR, "Error seeking to byte offset");
    g_dealloc(hr);
    return NULL;
  }

  size_t t = fread(hr, sizeof(data_record), 1, io->fp);
  if (t != 1) {
    g_error(BTREE_ERROR, "Error while reading data record");
    g_dealloc(hr);
    return NULL;
  }
  return hr;
}

void prepend_data_header(io_buf *io) {
  if (!io || !io->fp || !io->hr || !io->hr->free_rrn_address) {
    g_error(BTREE_ERROR, "Invalid input in prepend_data_header");
    return;
  }

  size_t free_rrn_len = strlen(io->hr->free_rrn_address) + 1,
         header_size = sizeof(u16) * 2 + free_rrn_len;

  if (io->hr->header_size != header_size) {
    io->hr->header_size = header_size;
  }

  fseek(io->fp, 0, SEEK_END);
  long original_file_size = ftell(io->fp);

  if (original_file_size <= 0) {
    g_error(BTREE_ERROR, "File is empty, no content to prepend");
    original_file_size = 0;
  }

  char *buffer = NULL;
  if (original_file_size > 0) {
    buffer = g_alloc(original_file_size);
    if (buffer == NULL) {
      g_error(BTREE_ERROR, "Memory allocation failed for file content");
      return;
    }

    fseek(io->fp, 0, SEEK_SET);
    if (fread(buffer, 1, original_file_size, io->fp) != original_file_size) {
      g_error(BTREE_ERROR, "Error reading original file content");
      g_dealloc(buffer);
      return;
    }
  }

  fclose(io->fp);
  io->fp = fopen(io->address, "wb");
  if (io->fp == NULL) {
    g_error(BTREE_ERROR, "Error reopening file in write mode");
    if (buffer)
      g_dealloc(buffer);
    return;
  }

  printf("free rrn address: %s\n", io->hr->free_rrn_address);
  fseek(io->fp, 0, SEEK_SET);
  if (fwrite(&io->hr->header_size, sizeof(u16), 1, io->fp) != 1 ||
      fwrite(&io->hr->record_size, sizeof(u16), 1, io->fp) != 1 ||
      fwrite(io->hr->free_rrn_address, free_rrn_len, 1, io->fp) != 1) {
    g_error(BTREE_ERROR, "Error writing header to file");
    if (buffer)
      g_dealloc(buffer);
    return;
  }

  if (buffer && original_file_size > 0) {
    if (fwrite(buffer, 1, original_file_size, io->fp) != original_file_size) {
      g_error(BTREE_ERROR, "Error writing original content after header");
    }
    g_dealloc(buffer);
  }

  g_info("Successfully written header: %hu %hu %s\n", io->hr->record_size, 
         io->hr->header_size, io->hr->free_rrn_address);

  fflush(io->fp);
}

void write_data_record(io_buf *io, void *d, u16 rrn) {
  if (!io || !io->fp || !d) {
    g_error(BTREE_ERROR, "Invalid input in write_data_record");
    return;
  }

  int byte_offset = io->hr->header_size + (io->hr->record_size * rrn);
  fseek(io->fp, byte_offset, SEEK_SET);
  size_t t = fwrite(d, sizeof(data_record), 1, io->fp);
  if (t != 1) {
    g_error(BTREE_ERROR, "Error while writing data record");
  }
}

void populate_header(data_header_record *hp, const char *file_name) {
  if (hp == NULL) {
    g_error(BTREE_ERROR, "Header pointer is NULL, cannot populate");
    return;
  }

  hp->record_size = sizeof(data_record);
  strcpy(hp->free_rrn_address, file_name);
  hp->free_rrn_address[strlen(file_name) + 1] = '\0';
  hp->header_size = strlen(file_name) + 1 + sizeof(u16) * 2;
}

void load_file(io_buf *io, char *file_name, const char *type) {
  if (!file_name) {
    g_error(BTREE_ERROR, "ERROR: no file");
    return;
  }

  if (io->fp != NULL) {
    g_error(BTREE_ERROR, "Buffer already filled. Closing logical link");
    if (fclose(io->fp) != 0) {
      g_error(BTREE_ERROR, "Failed to close %s", file_name);
      return;
    }
    io->fp = NULL;
  }

  sprintf(io->address, "%s", file_name);

  g_debug(DISK_STATUS, "Loading file: %s\n", file_name);

  io->fp = fopen(io->address, "r+b");
  if (io->fp == NULL) {
    g_error(BTREE_ERROR, "Error opening file");
    return;
  }

  if (strcmp(type, "index") == 0) {
    if (io->br == NULL) {
      io->br = g_alloc(sizeof(index_header_record));
      if (io->br == NULL) {
        g_error(BTREE_ERROR, "Memory allocation failed for index_header_record");
        return;
      }
    }
    load_index_header(io);
    if (!io->br || !io->hr)
      exit(-1);
  }

  if (strcmp(type, "data") == 0) {
    if (io->hr == NULL) {
      io->hr = g_alloc(sizeof(data_header_record));
      if (io->hr == NULL) {
        g_error(BTREE_ERROR, "Memory allocation failed for data_header_record");
        fclose(io->fp);
        return;
      }
    }
    load_data_header(io);
  }
  if (strcmp(type, "data") == 0 &&
      (io->hr->record_size == 0 || io->hr->header_size == 0)) {
    g_error(BTREE_ERROR, "Error: one or more inputs in data_header_record are 0");
    fclose(io->fp);
    return;
  }

  if (strcmp(type, "index") == 0 &&
      (io->br->btree_node_size == 0 || io->br->header_size == 0)) {
    g_error(BTREE_ERROR, "Error: one or more inputs in index_header_record are 0");
    fclose(io->fp);
    return;
  }

  g_info("Loaded %s", file_name);
}

void create_data_file(io_buf *io, char *file_name) {
  if (!io || !file_name) {
    g_error(BTREE_ERROR, "Invalid IO buffer or file name");
    return;
  }

  strcpy(io->address, file_name);

  if (io->fp != NULL) {
    g_error(BTREE_ERROR, "File already open");
    return;
  }

  int prepend = 0;
  char list_name[MAX_ADDRESS];
  strcpy(list_name, file_name);
  char *dot = strrchr(list_name, '.');
  if (dot)
    strcpy(dot, ".hlp");

  io->fp = fopen(io->address, "r+b");
  if (!io->fp) {
    g_error(BTREE_ERROR, "!!Error opening file: %s. Creating it...\n", io->address);
    io->fp = fopen(io->address, "wb");
    if (!io->fp) {
      g_error(BTREE_ERROR, "Error creating file");
      return;
    }
    fclose(io->fp);
    io->fp = fopen(io->address, "r+b");
    prepend = 1;
    populate_header(io->hr, list_name);
  } else {
    if (io->hr == NULL) {
      io->hr = g_alloc(sizeof(data_header_record));
      if (io->hr == NULL) {
        g_error(BTREE_ERROR, "Memory allocation failed for data_header_record");
        fclose(io->fp);
        return;
      }
    }

    load_data_header(io);

    if (io->hr->record_size != sizeof(data_record)) {
      puts("needs to prepend");
      prepend = 1;
      populate_header(io->hr, list_name);
    }
  }

  if (prepend == 1) {
    populate_header(io->hr, list_name);
    prepend_data_header(io);
  }

  g_info("Data file created successfully");
}

void clear_io_buf(io_buf *io) {
  if (!io)
    return;

  if (io->fp) {
    fclose(io->fp);
    io->fp = NULL;
  }

  if (io->hr) {
    g_dealloc(io->hr->free_rrn_address);
    g_dealloc(io->hr);
    io->hr = NULL;
  }

  if (io->br) {
    g_dealloc(io->br->free_rrn_address);
    g_dealloc(io->br);
    io->br = NULL;
  }

  g_dealloc(io);
}
