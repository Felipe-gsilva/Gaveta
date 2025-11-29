#include "b_tree.h"
#include "../../App.h"

extern App app;

btree_node **g_allocated;
u16 g_n = 0;

queue *alloc_queue(void) {
  queue *root = malloc(sizeof(queue));
  if (!root) {
    puts("!!Error: Memory allocation failed");
    return NULL;
  }
  root->next = NULL;
  root->page = NULL;
  root->counter = 0;
  if (app.debug)
    puts("@Allocated queue");
  return root;
}

void clear_queue(queue *q) {
  if (!q) {
    puts("!!Error: NULL queue pointer\n");
    return;
  }
  queue *current = q->next;
  queue *next_node;
  while (current) {
    next_node = current->next;
    if (current->page) {
      current->page = NULL;
    }
    free(current);
    current = next_node;
  }
  q->next = NULL;
  q->counter = 0;

  if (app.debug) {
    puts("@Queue cleared");
  }
}
void print_queue(queue *q) {
  if (!q) {
    fprintf(stderr, "!!Error: NULL queue pointer\n");
    return;
  }

  if (!q->next) {
    fprintf(stderr, "!!Error: Empty queue\n");
    return;
  }

  printf("Queue contents:\n");
  queue *current = q->next;
  int node_count = 0;

  while (current) {
    if (!current->page) {
      fprintf(stderr, "!!Error: NULL page in queue node %d\n", node_count);
      current = current->next;
      continue;
    }

    printf("Node %d (RRN: %d) Keys: ", node_count, current->page->rrn);

    print_btree_node(current->page);
    printf("\n");

    current = current->next;
    node_count++;
  }

  if (app.debug) {
    printf("Total nodes in queue: %d\n", node_count);
  }
}

void push_btree_node(b_tree_buf *b, btree_node *p) {
  if (!b || !b->q || !p) {
    puts("!!Error: NULL queue pointer or page");
    return;
  }

  if (queue_search(b->q, p->rrn)) {
    if (app.debug)
      puts("@Page already in queue");
    return;
  }

  if (b->q->counter >= P) {
    pop_btree_node(b);
  }

  queue *new_node = malloc(sizeof(queue));
  if (!new_node) {
    puts("!!Error: Memory allocation failed");
    return;
  }

  new_node->page = p;
  new_node->next = b->q->next;
  b->q->next = new_node;
  b->q->counter++;

  if (app.debug)
    puts("@Pushed page onto queue");
}

btree_node *pop_btree_node(b_tree_buf *b) {
  if (!b->q || b->q->next == NULL) {
    puts("!!Error: NULL or Empty queue pointer");
    return NULL;
  }

  queue *head = b->q->next;
  btree_node *page = head->page;

  b->q->next = head->next;
  b->q->counter--;

  if (app.debug)
    puts("@Popped from queue");

  free(head);
  return page;
}
btree_node *queue_search(queue *q, u16 rrn) {
  if (!q)
    return NULL;

  queue *current = q->next;
  while (current) {
    if (current->page && current->page->rrn == rrn) {
      if (app.debug) {
        printf("@Page with RRN %hu found in queue\n", rrn);
      }
      return current->page;
    }
    current = current->next;
  }
  return NULL;
}


b_tree_buf *alloc_tree_buf(void) {
  b_tree_buf *b = malloc(sizeof(b_tree_buf));
  if (!b) {
    puts("!!Could not allocate b_tree_buf_BUFFER");
    return NULL;
  }

  b->root = NULL;
  b->io = alloc_io_buf();
  if (!b->io) {
    free(b);
    puts("!!Could not allocate io_buf");
    return NULL;
  }

  b->q = alloc_queue();
  if (!b->q) {
    free(b->io);
    free(b);
    puts("!!Could not allocate queue");
    return NULL;
  }

  b->i = alloc_ilist();
  if (!b->i) {
    free(b->q);
    free(b->io);
    free(b);
    puts("!!Could not allocate ilist");
    return NULL;
  }

  if (app.debug)
    puts("@Allocated b_tree_buf_BUFFER");
  return b;
}

void clear_tree_buf(b_tree_buf *b) {
  if (b) {
    clear_ilist(b->i);
    clear_queue(b->q);
    clear_io_buf(b->io);
    if (b->root) {
      btree_node *q_page = queue_search(b->q, b->root->rrn);
      if (!q_page) {
        b->root = NULL;
      }
    }
    free(b);
    b = NULL;
  }
  if (app.debug)
    puts("@B_TREE_BUFFER cleared");
}

void populate_index_header(index_header_record *bh, const char *file_name) {
  if (bh == NULL) {
    puts("!!Header pointer is NULL, cannot populate");
    return;
  }

  bh->page_size = sizeof(btree_node);
  bh->root_rrn = 0;
  strcpy(bh->free_rrn_address, file_name);
  bh->free_rrn_address[strlen(file_name) + 1] = '\0';
  bh->header_size = (sizeof(u16) * 3) + strlen(file_name) + 1;
}

void build_tree(b_tree_buf *b, io_buf *data, int n) {
  if (!b || !data) {
    puts("!!Invalid parameters");
    return;
  }
  if (!b->i) {
    load_list(b->i, b->io->br->free_rrn_address);
    if (b->i && app.debug) {
      puts("@Loaded rrn list");
    }
  }
  data_record *d;
  for (int i = 0; i < n; i++) {
    d = load_data_record(data, i);
    if (!d) {
      printf("!!Failed to load record %d\n", i);
      continue;
    }
    if (app.debug)
      print_data_record(d);

    btree_status status = b_insert(b, data, d, i);
    if ((status != BTREE_SUCCESS) && (status != BTREE_INSERTED_IN_PAGE)) {
      printf("!!Failed to insert record %d, error: %d\n", i - 1, status);
      exit(0);
    }
  }
  if (d)
    free(d);

  if (app.debug) {
    puts("@Built tree");
  }
}

btree_node *load_page(b_tree_buf *b, u16 rrn) {
  if (!b || !b->io) {
    puts("!!Error: invalid parameters");
    return NULL;
  }

  btree_node *page = queue_search(b->q, rrn);
  if (page) {
    if (app.debug)
      puts("@Page found in queue");
    return page;
  }

  page = alloc_btree_node();
  if (!page)
    return NULL;

  page->rrn = rrn;

  size_t byte_offset =
      (size_t)(b->io->br->header_size) + ((size_t)(b->io->br->page_size) * rrn);

  if (fseek(b->io->fp, byte_offset, SEEK_SET) != 0) {
    free(page);
    return NULL;
  }

  size_t bytes_read = fread(page, 1, b->io->br->page_size, b->io->fp);
  if (bytes_read != b->io->br->page_size) {
    free(page);
    return NULL;
  }

  push_btree_node(b, page);

  return page;
}

int write_root_rrn(b_tree_buf *b, u16 rrn) {
  if (!b) {
    puts("!!Error: NULL b_tree_buf");
    return BTREE_ERROR_IO;
  }

  b->io->br->root_rrn = rrn;

  fseek(b->io->fp, 0, SEEK_SET);
  size_t flag = fwrite(&rrn, sizeof(u16), 1, b->io->fp);
  if (flag != 1) {
    puts("!!Error: Could not update root rrn");
    exit(-1);
  }

  fflush(b->io->fp);

  return BTREE_SUCCESS;
}

void b_update(b_tree_buf *b, io_buf *data, free_rrn_list *ld,
              const char *placa) {}

btree_node *b_search(b_tree_buf *b, const char *s, u16 *return_pos) {
  if (!b || !b->root || !s)
    return NULL;

  key k;
  strncpy(k.id, s, 1); // TODO key size minus null terminator
  k.id[/* TODO */ 0] = '\0';

  btree_node *found_page = NULL;
  *return_pos = search_key(b, b->root, k, return_pos, &found_page);

  if (found_page->leaf)
    return found_page;

  *return_pos = (u16)-1;
  return NULL;
}

void b_range_search(b_tree_buf *b, io_buf *data, key_range *range) {
  if (!b || !range || !b->root) {
    puts("!!Invalid parameters for range search");
    return;
  }

  btree_node *curr = b->root;
  while (!curr->leaf) {
    int i;
    for (i = 0; i < curr->keys_num; i++) {
      if (strcmp(range->start_id, curr->keys[i].id) < 0) {
        break;
      }
    }
    curr = load_page(b, curr->children[i]);
    if (!curr) {
      puts("!!Error loading page during range search");
      return;
    }
  }

  bool found_any = false;
  while (curr) {
    if (curr->keys_num > 0 && strcmp(curr->keys[0].id, range->end_id) > 0) {
      break;
    }

    for (int i = 0; i < curr->keys_num; i++) {
      if (strcmp(curr->keys[i].id, range->end_id) > 0) {
        break;
      }

      if (strcmp(curr->keys[i].id, range->start_id) >= 0) {
        found_any = true;
        data_record *record =
            load_data_record(data, curr->keys[i].data_register_rrn);
        if (record) {
          print_data_record(record);
          free(record);
        }
      }
    }

    if (curr->next_leaf == (u16)-1) {
      break;
    }

    btree_node *next = load_page(b, curr->next_leaf);
    curr = next;
  }

  if (!found_any) {
    puts("Nenhum registro encontrado no intervalo especificado.");
  }
}

int search_in_page(btree_node *p, key key, int *return_pos) {
  if (!p) {
    puts("!!Error: no page");
    return BTREE_ERROR_INVALID_PAGE;
  }

  for (int i = 0; i < p->keys_num; i++) {
    if (p->keys[i].id[0] == '\0') {
      *return_pos = i;
      return BTREE_NOT_FOUND_KEY;
    }

    if (app.debug)
      printf("page key id: %s\t key id: %s\n", p->keys[i].id, key.id);
    if (strcmp(p->keys[i].id, key.id) == 0) {
      puts("@Curr key was found");
      *return_pos = i;
      return BTREE_FOUND_KEY;
    }

    if (strcmp(p->keys[i].id, key.id) > 0) {
      *return_pos = i;
      if (app.debug)
        puts("@Curr key is greater than the new one");
      return BTREE_NOT_FOUND_KEY;
    }
  }

  *return_pos = p->keys_num;
  return BTREE_NOT_FOUND_KEY;
}

u16 search_key(b_tree_buf *b, btree_node *p, key k, u16 *found_pos,
               btree_node **return_page) {
  if (!p)
    return (u16)-1;

  int pos;
  int result = search_in_page(p, k, &pos);

  if (app.debug) {
    printf("page key id: %s     key id: %s\n",
           p->keys_num > 0 ? p->keys[0].id : "", k.id);
  }

  if (result == BTREE_FOUND_KEY) {
    *found_pos = pos;
    *return_page = p;
    return pos;
  }

  if (p->leaf) {
    *return_page = p;
    *found_pos = pos;
    return (u16)-1;
  }

  btree_node *next = load_page(b, p->children[pos]);
  if (!next)
    return (u16)-1;

  u16 ret = search_key(b, next, k, found_pos, return_page);

  if (next != b->root && !queue_search(b->q, next->rrn)) {
    free(next);
  }

  return ret;
}

void populate_key(key *k, data_record *d, u16 rrn) {
  if (!k || !d)
    return;

  strncpy(k->id, d->key, 0); // TODO key size minus null terminator
  k->data_register_rrn = rrn;

  if (app.debug) {
    printf("@Populated key with ID: %s and data RRN: %hu\n", k->id,
           k->data_register_rrn);
  }
}

btree_status insert_in_page(btree_node *p, key k, btree_node *r_child, int pos) {
  if (!p)
    return BTREE_ERROR_INVALID_PAGE;

  if (app.debug) {
    printf("Current state - keys: %d, children: %d, inserting at pos: %d\n",
           p->keys_num, p->child_num, pos);
  }

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

  if (app.debug) {
    printf("After insertion - keys: %d, children: %d\n", p->keys_num,
           p->child_num);
    printf("Inserted key with data RRN: %hu\n", k.data_register_rrn);
  }

  return BTREE_INSERTED_IN_PAGE;
}

btree_status b_insert(b_tree_buf *b, io_buf *data, data_record *d, u16 rrn) {
  if (!b || !data || !d)
    return BTREE_ERROR_INVALID_PAGE;

  key new_key;
  populate_key(&new_key, d, rrn);

  if (!b->root) {
    b->root = alloc_btree_node();
    if (!b->root)
      return BTREE_ERROR_MEMORY;

    b->root->rrn = get_free_rrn(b->i);
    if (b->root->rrn < 0) {
      free(b->root);
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
    btree_node *new_root = alloc_btree_node();
    if (!new_root)
      return BTREE_ERROR_MEMORY;

    new_root->rrn = get_free_rrn(b->i);
    if (new_root->rrn < 0) {
      free(new_root);
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

btree_status b_split(b_tree_buf *b, btree_node *p, btree_node **r_child, key *promo_key,
                     key *incoming_key, bool *promoted) {
  if (!b || !p || !r_child || !promo_key || !incoming_key)
    return BTREE_ERROR_INVALID_PAGE;

  key temp_keys[ORDER];
  u16 temp_children[ORDER + 1];

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

  btree_node *new_page = alloc_btree_node();
  if (!new_page)
    return BTREE_ERROR_MEMORY;

  new_page->rrn = get_free_rrn(b->i);
  if (new_page->rrn == (u16)-1) {
    free(new_page);
    return BTREE_ERROR_IO;
  }

  int split = (ORDER - 1) / 2;

  if (p->leaf) {
    p->keys_num = split + 1;
    new_page->keys_num = ORDER - (split + 1);
    new_page->leaf = true;

    for (int i = 0; i < p->keys_num; i++) {
      p->keys[i] = temp_keys[i];
    }

    for (int i = 0; i < new_page->keys_num; i++) {
      new_page->keys[i] = temp_keys[i + split + 1];
    }

    new_page->next_leaf = p->next_leaf;
    p->next_leaf = new_page->rrn;

    *promo_key = new_page->keys[0];
  } else {
    p->keys_num = split;
    new_page->keys_num = ORDER - split - 1;
    new_page->leaf = false;

    for (int i = 0; i < p->keys_num; i++) {
      p->keys[i] = temp_keys[i];
    }

    *promo_key = temp_keys[split];

    for (int i = 0; i < new_page->keys_num; i++) {
      new_page->keys[i] = temp_keys[i + split + 1];
    }

    for (int i = 0; i <= p->keys_num; i++) {
      p->children[i] = temp_children[i];
    }

    for (int i = 0; i <= new_page->keys_num; i++) {
      new_page->children[i] = temp_children[i + split + 1];
    }

    p->child_num = p->keys_num + 1;
    new_page->child_num = new_page->keys_num + 1;
  }

  btree_status status;
  if ((status = write_index_record(b, p)) != BTREE_SUCCESS) {
    free(new_page);
    return status;
  }

  if ((status = write_index_record(b, new_page)) != BTREE_SUCCESS) {
    free(new_page);
    return status;
  }

  *r_child = new_page;
  *promoted = true;

  return BTREE_PROMOTION;
}
btree_status insert_key(b_tree_buf *b, btree_node *p, key k, key *promo_key,
                        btree_node **r_child, bool *promoted) {
  if (!b || !promo_key || !p)
    return BTREE_ERROR_INVALID_PAGE;

  int pos;
  btree_status status = search_in_page(p, k, &pos);
  if (status == BTREE_FOUND_KEY)
    return BTREE_ERROR_DUPLICATE;

  if (!p->leaf) {
    btree_node *child = load_page(b, p->children[pos]);
    if (!child)
      return BTREE_ERROR_IO;

    key temp_key;
    btree_node *temp_child = NULL;
    status = insert_key(b, child, k, &temp_key, &temp_child, promoted);

    if (child != b->root && !queue_search(b->q, child->rrn)) {
      clear_btree_node(child);
    }

    if (status == BTREE_PROMOTION) {
      k = temp_key;
      *r_child = temp_child;
      if (p->keys_num < ORDER - 1) {
        *promoted = false;
        status = insert_in_page(p, k, temp_child, pos);
        if (status == BTREE_INSERTED_IN_PAGE) {
          return write_index_record(b, p);
        }
        return status;
      }
      return b_split(b, p, r_child, promo_key, &k, promoted);
    }
    return status;
  }

  if (p->keys_num < ORDER - 1) {
    *promoted = false;
    status = insert_in_page(p, k, NULL, pos);
    if (status == BTREE_INSERTED_IN_PAGE) {
      return write_index_record(b, p);
    }
    return status;
  }

  return b_split(b, p, r_child, promo_key, &k, promoted);
}

btree_status b_remove(b_tree_buf *b, io_buf *data, char *key_id) {
  if (!b || !b->root || !data || !key_id)
    return BTREE_ERROR_INVALID_PAGE;

  if (app.debug)
    printf("@Removing key: %s\n", key_id);

  u16 pos;
  btree_node *p = b_search(b, key_id, &pos);
  if (!p || strcmp(p->keys[pos].id, key_id) != 0) {
    if (app.debug)
      puts("@Key not found");
    return BTREE_NOT_FOUND_KEY;
  }

  if (p->leaf) {
    if (app.debug)
      printf("@Removing key from leaf btree_node RRN: %hu at position: %hu\n", p->rrn,
             pos);
    u16 data_rrn = p->keys[pos].data_register_rrn;

    for (int i = pos; i < p->keys_num - 1; i++)
      p->keys[i] = p->keys[i + 1];
    p->keys_num--;

    if (data_rrn != (u16)-1) {
      if (fseek(data->fp,
                data->hr->header_size + (data_rrn * sizeof(data_record)),
                SEEK_SET) == 0) {
        data_record empty_record;
        memset(&empty_record, '*', sizeof(data_record));
        fwrite(&empty_record, sizeof(data_record), 1, data->fp);
        fflush(data->fp);
        insert_list(b->i, data_rrn);
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

    if (p != b->root && p->keys_num < (ORDER - 1) / 2) {
      if (app.debug)
        puts("@Leaf underflow detected");

      btree_node *left = get_sibling(b, p, true);
      if (left && left->keys_num > (ORDER - 1) / 2) {
        return redistribute(b, left, p, true);
      }

      btree_node *right = get_sibling(b, p, false);
      if (right && right->keys_num > (ORDER - 1) / 2) {
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

  if (app.debug)
    puts("@Key found in internal node - not removing");
  return BTREE_SUCCESS;
}

btree_status redistribute(b_tree_buf *b, btree_node *donor, btree_node *receiver,
                          bool from_left) {
  if (!b || !donor || !receiver)
    return BTREE_ERROR_INVALID_PAGE;

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

btree_status merge(b_tree_buf *b, btree_node *left, btree_node *right) {
  if (!b || !left || !right)
    return BTREE_ERROR_INVALID_PAGE;

  for (int i = 0; i < right->keys_num; i++) {
    left->keys[left->keys_num + i] = right->keys[i];
  }
  left->keys_num += right->keys_num;

  left->next_leaf = right->next_leaf;

  btree_status status = write_index_record(b, left);
  if (status < 0)
    return status;

  insert_list(b->i, right->rrn);

  return BTREE_SUCCESS;
}

btree_node *get_sibling(b_tree_buf *b, btree_node *p, bool left) {
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
    return load_page(b, parent->children[pos - 1]);
  } else if (!left && pos < parent->child_num - 1) {
    return load_page(b, parent->children[pos + 1]);
  }

  return NULL;
}

void print_page(btree_node *p) {
  if (!p) {
    puts("!!Página nula");
    return;
  }

  printf("RRN: %hu | Folha: %d | Chaves: %d | Filhos: %d\n", p->rrn, p->leaf,
         p->keys_num, p->child_num);

  printf("Chaves: ");
  for (int i = 0; i < p->keys_num; i++) {
    printf("[%s]", p->keys[i].id);
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
    puts("!!NULL file");
    return BTREE_ERROR_IO;
  }
  if (io->br == NULL) {
    io->br = malloc(sizeof(index_header_record));
    if (io->br == NULL) {
      puts("!!Memory allocation error");
      return BTREE_ERROR_MEMORY;
    }
  }
  if (io->br->page_size == 0) {
    puts("!!Error: btree_node size == 0");
    return BTREE_ERROR_INVALID_PAGE;
  }

  size_t free_rrn_len = strlen(io->br->free_rrn_address) + 1;
  io->br->header_size = sizeof(u16) * 3 + free_rrn_len;

  fseek(io->fp, 0, SEEK_SET);

  if (fwrite(&io->br->root_rrn, sizeof(u16), 1, io->fp) != 1) {
    puts("!!Error while writing root_rrn");
    return BTREE_ERROR_IO;
  }

  if (fwrite(&io->br->page_size, sizeof(u16), 1, io->fp) != 1) {
    puts("!!Error while writing page_size");
    return BTREE_ERROR_IO;
  }

  if (fwrite(&io->br->header_size, sizeof(u16), 1, io->fp) != 1) {
    puts("!!Error while writing size");
    return BTREE_ERROR_IO;
  }

  if (fwrite(io->br->free_rrn_address, free_rrn_len, 1, io->fp) != 1) {
    puts("!!Error while writing free_rrn_address");
    return BTREE_ERROR_IO;
  }

  if (app.debug) {
    printf("@Successfully written on index: root_rrn: %hu, page_size: %hu, "
           "size: %hu, "
           "free_rrn_address: %s\n",
           io->br->root_rrn, io->br->page_size, io->br->header_size,
           io->br->free_rrn_address);
  }

  fflush(io->fp);
  return BTREE_SUCCESS;
}

void load_index_header(io_buf *io) {
  if (!io || !io->fp) {
    puts("!!Invalid IO buffer or file pointer");
    return;
  }

  if (!io->br) {
    io->br = malloc(sizeof(index_header_record));
    if (!io->br) {
      puts("!!Memory allocation error");
      return;
    }
    memset(io->br, 0, sizeof(index_header_record));
  }

  fseek(io->fp, 0, SEEK_SET);

  if (fread(&io->br->root_rrn, sizeof(u16), 1, io->fp) != 1) {
    puts("!!Error reading root_rrn");
    return;
  }

  if (fread(&io->br->page_size, sizeof(u16), 1, io->fp) != 1) {
    puts("!!Error reading page_size");
    return;
  }

  if (fread(&io->br->header_size, sizeof(u16), 1, io->fp) != 1) {
    puts("!!Error reading size");
    return;
  }

  printf("root_rrn: %hu, page_size: %hu, size: %hu\n", io->br->root_rrn,
         io->br->page_size, io->br->header_size);

  size_t rrn_len = io->br->header_size - (3 * sizeof(u16));

  io->br->free_rrn_address = malloc(rrn_len + 1);
  if (!io->br->free_rrn_address) {
    puts("!!Memory allocation error for free_rrn_address");
    return;
  }

  if (fread(io->br->free_rrn_address, rrn_len, 1, io->fp) != 1) {
    puts("!!Error reading free_rrn_address");
    free(io->br->free_rrn_address);
    io->br->free_rrn_address = NULL;
    return;
  }
  io->br->free_rrn_address[rrn_len] = '\0';

  if (app.debug) {
    puts("@Index header Record Loaded");
    printf("-->index_header: root_rrn: %hu page_size: %hu size: %hu "
           "free_rrn_list: %s\n",
           io->br->root_rrn, io->br->page_size, io->br->header_size,
           io->br->free_rrn_address);
  }
}

btree_status write_index_record(b_tree_buf *b, btree_node *p) {
  if (!b || !b->io || !p)
    return BTREE_ERROR_IO;

  if (app.debug) {
    puts("////////");
    puts("@Writting following btree_node: ");
    print_page(p);
  }

  int byte_offset = b->io->br->header_size + (b->io->br->page_size * p->rrn);

  if (fseek(b->io->fp, byte_offset, SEEK_SET)) {
    puts("!!Error: could not fseek");
    return BTREE_ERROR_IO;
  }

  size_t written = fwrite(p, b->io->br->page_size, 1, b->io->fp);
  if (written != 1) {
    puts("!!Error: could not write btree_node");
    return BTREE_ERROR_IO;
  }

  fflush(b->io->fp);

  if (app.debug) {
    printf("@Successfully wrote btree_node %hu at offset %d\n", p->rrn, byte_offset);
  }

  if (!queue_search(b->q, p->rrn)) {
    push_btree_node(b, p);
  }

  return BTREE_SUCCESS;
}

void create_index_file(io_buf *io, const char *file_name) {
  if (!io || !file_name) {
    puts("!!Invalid io buffer or file name");
    return;
  }

  strcpy(io->address, file_name);

  if (io->hr == NULL) {
    io->hr = malloc(sizeof(data_header_record));
    if (io->hr == NULL) {
      puts("!!Memory allocation failed for data_header_record");
      return;
    }
  }

  if (io->br == NULL) {
    free(io->hr);
    io->hr = NULL;
    io->br = malloc(sizeof(index_header_record));
    if (io->br == NULL) {
      puts("!!Memory allocation failed for index_header_record");
      return;
    }
    io->br->free_rrn_address = malloc(sizeof(char) * MAX_ADDRESS);
    if (io->br->free_rrn_address == NULL) {
      puts("!!Memory allocation failed for free_rrn_address");
      return;
    }
  }

  if (io->fp != NULL) {
    puts("!!File already opened");
    return;
  }

  printf("Loading file: %s\n", io->address);
  io->fp = fopen(io->address, "r+b");
  if (!io->fp) {
    if (app.debug)
      printf("!!Error opening file: %s. Creating it...\n", io->address);
    io->fp = fopen(io->address, "wb");
    if (io->fp) {
      fclose(io->fp);
    }
    io->fp = fopen(io->address, "r+b");
    if (!io->fp) {
      puts("!!Failed to create and open file");
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

  if (app.debug) {
    puts("@Index file created successfully");
  }
}

btree_node *alloc_page(void) {
  btree_node *p = NULL;
  if (posix_memalign((void **)&p, sizeof(void *), sizeof(btree_node)) != 0) {
    puts("!!Erro: falha na alocação da página");
    return NULL;
  }

  memset(p, 0, sizeof(btree_node));
  p->leaf = true;
  p->next_leaf = (u16)-1;

  for (int i = 0; i < ORDER; i++) {
    p->children[i] = (u16)-1;
  }

  return p;
}

void clear_all_pages(void) {
  if (!g_allocated)
    return;

  if (app.debug)
    puts("clearing pages:");

  for (int i = 0; i < g_n; i++) {
    if (g_allocated[i]) {
      if (app.debug)
        printf("%hu\t", g_allocated[i]->rrn);
      free(g_allocated[i]);
      g_allocated[i] = NULL;
    }
  }

  free(g_allocated);
  g_allocated = NULL;
  g_n = 0;

  if (app.debug)
    puts("");
}

void clear_page(btree_node *btree_node) {
  if (btree_node) {
    free(btree_node);
    if (app.debug)
      puts("@Successfully freed btree_node");
    return;
  }
  puts("Error while freeing btree_node");
}

void track_page(btree_node *p) {
  if (!p)
    return;

  if (!g_allocated) {
    g_allocated = malloc(sizeof(btree_node *));
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

btree_node *find_parent(b_tree_buf *b, btree_node *current, btree_node *target) {
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
      btree_node *child = load_page(b, current->children[i]);
      if (!child)
        continue;

      btree_node *result = find_parent(b, child, target);

      if (child != b->root && !queue_search(b->q, child->rrn)) {
        clear_page(child);
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

bool rrn_exists(u16 A[], int n, int rrn) {
  if (!A || n <= 0)
    return false;

  int left = 0;
  int right = n - 1;

  while (left <= right) {
    int mid = (left + right) / 2;
    if (A[mid] == rrn)
      return true;
    if (A[mid] < rrn)
      left = mid + 1;
    else
      right = mid - 1;
  }
  return false;
}

void write_rrn_list_to_file(free_rrn_list *i) {
  if (!i || !i->io->fp)
    return;

  fseek(i->io->fp, 0, SEEK_SET);
  if (i->n > 0) {
    if (fwrite(&i->n, sizeof(u16), 1, i->io->fp) != 1) {
      puts("!!Error: Failed to write RRN count");
      return;
    }

    size_t written = fwrite(i->free_rrn, sizeof(u16), i->n, i->io->fp);
    if (written != i->n) {
      printf("!!Error: Expected to write %d elements, but wrote %zu\n", i->n,
             written);
      return;
    }
    return;
  }

  i->n = 0;
  if (fwrite(&i->n, sizeof(u16), 1, i->io->fp) != 1) {
    puts("!!Error: Failed to write empty RRN count");
  }

  fflush(i->io->fp);
}

free_rrn_list *alloc_ilist(void) {
  free_rrn_list *i = malloc(sizeof(free_rrn_list));
  if (!i)
    exit(-1);
  i->io = alloc_io_buf();
  i->n = 0;
  i->free_rrn = NULL;
  return i;
}

void clear_ilist(free_rrn_list *i) {
  if (!i)
    return;

  if (i->io && i->io->fp) {
    fclose(i->io->fp);
    i->io->fp = NULL;
  }

  free(i->free_rrn);
  i->free_rrn = NULL;

  clear_io_buf(i->io);
  free(i);
}

void load_list(free_rrn_list *i, char *s) {
  if (!i || !s) {
    puts("!!Error: Invalid parameters");
    return;
  }

  if (i->free_rrn) {
    free(i->free_rrn);
    i->free_rrn = NULL;
  }

  if (!i->io) {
    i->io = alloc_io_buf();
    if (!i->io) {
      puts("!!Error: Failed to allocate IO buffer");
      return;
    }
  }

  strcpy(i->io->address, s);
  i->io->address[strlen(s)] = '\0';

  i->io->fp = fopen(i->io->address, "r+b");
  if (!i->io->fp) {
    printf("Creating new RRN list file: %s\n", s);
    i->io->fp = fopen(i->io->address, "wb");
    if (!i->io->fp) {
      printf("!!Error: Cannot create file %s\n", s);
      return;
    }
    i->n = 1;
    i->free_rrn = malloc(sizeof(u16));
    if (!i->free_rrn) {
      puts("!!Error: Failed to allocate RRN list");
      fclose(i->io->fp);
      return;
    }
    i->free_rrn[0] = 0;
    fwrite(&i->n, sizeof(u16), 1, i->io->fp);
    fwrite(i->free_rrn, sizeof(u16), i->n, i->io->fp);
    fclose(i->io->fp);
    i->io->fp = fopen(i->io->address, "r+b");
  }

  fseek(i->io->fp, 0, SEEK_SET);
  size_t read = fread(&i->n, sizeof(u16), 1, i->io->fp);
  if (read != 1) {
    i->n = 1;
    i->free_rrn = malloc(sizeof(u16));
    if (!i->free_rrn) {
      puts("!!Error: Failed to allocate RRN list");
      return;
    }
    i->free_rrn[0] = 0;
    fseek(i->io->fp, 0, SEEK_SET);
    fwrite(&i->n, sizeof(u16), 1, i->io->fp);
    fwrite(i->free_rrn, sizeof(u16), i->n, i->io->fp);
  } else if (i->n > 0) {
    i->free_rrn = load_rrn_list(i);
    if (!i->free_rrn) {
      i->n = 1;
      i->free_rrn = malloc(sizeof(u16));
      if (!i->free_rrn) {
        puts("!!Error: Failed to allocate RRN list");
        return;
      }
      i->free_rrn[0] = 0;
      fseek(i->io->fp, 0, SEEK_SET);
      fwrite(&i->n, sizeof(u16), 1, i->io->fp);
      fwrite(i->free_rrn, sizeof(u16), i->n, i->io->fp);
    }
  }

  fflush(i->io->fp);
  if (app.debug)
    printf("@Loaded RRN list with %d entries\n", i->n);
}

u16 *load_rrn_list(free_rrn_list *i) {
  if (!i->io->fp || i->n == 0)
    return NULL;

  u16 *list = malloc(sizeof(u16) * i->n);
  if (!list) {
    puts("!!Error: memory allocation failed");
    return NULL;
  }

  fseek(i->io->fp, sizeof(u16), SEEK_SET);
  size_t read = fread(list, sizeof(u16), i->n, i->io->fp);

  if (read != i->n) {
    free(list);
    printf("!!Error: Expected to read %d elements, but read %zu\n", i->n, read);
    return NULL;
  }

  if (app.debug) {
    for (int j = 0; j < i->n; j++)
      printf("i->list[%d]: %hu\t", j, list[j]);
    puts("");
  }
  return list;
}

u16 get_free_rrn(free_rrn_list *i) {
  if (!i || !i->io->fp) {
    puts("!!Error: Invalid list or file pointer");
    exit(1);
  }

  free(i->free_rrn);
  i->free_rrn = load_rrn_list(i);

  if (!i->free_rrn || i->n == 0) {
    puts("!!Error: No free RRNs available; initializing with default");
    i->n = 1;
    i->free_rrn = malloc(sizeof(u16));
    if (!i->free_rrn) {
      puts("!!Error: Failed to allocate RRN list");
      exit(1);
    }
    i->free_rrn[0] = 0;
    write_rrn_list_to_file(i);
    return 0;
  }

  int rrn = i->free_rrn[0];
  i->n--;

  if (i->n > 0)
    memmove(i->free_rrn, i->free_rrn + 1, sizeof(u16) * i->n);

  if (i->n < 1) {
    u16 new_rrn = rrn + 1;
    while (rrn_exists(i->free_rrn, i->n, new_rrn))
      new_rrn++;

    u16 *new_list = realloc(i->free_rrn, (i->n + 1) * sizeof(u16));
    if (!new_list) {
      puts("!!Error: Failed to reallocate RRN list");
    }
    i->free_rrn = new_list;
    i->free_rrn[i->n] = new_rrn;
    i->n++;
  }

  sort_list(i->free_rrn, i->n);
  write_rrn_list_to_file(i);
  return rrn;
}

u16 get_last_free_rrn(free_rrn_list *i) {
  if (!i || !i->io->fp) {
    puts("!!Error: invalid list or file pointer");
    return (u16)-1;
  }

  if (!i->free_rrn)
    i->free_rrn = load_rrn_list(i);

  if (!i->free_rrn || i->n == 0) {
    puts("!!Error: no free RRNs available");
    return (u16)-1;
  }
  return i->free_rrn[i->n - 1];
}

void insert_list(free_rrn_list *i, int rrn) {
  if (!i || !i->io->fp) {
    puts("!!Error: NULL rrn list or file pointer");
    return;
  }

  if (app.debug)
    printf("Current count before insertion: %d\n", i->n);
  if (rrn_exists(i->free_rrn, i->n, rrn)) {
    if (app.debug)
      printf("@RRN %d already exists in the list\n", rrn);
    return;
  }

  u16 *new_list = realloc(i->free_rrn, (i->n + 1) * sizeof(u16));
  if (!new_list) {
    puts("!!Error: Memory allocation failed");
    return;
  }

  i->free_rrn = new_list;
  i->free_rrn[i->n++] = rrn;
  if (app.debug)
    printf("New count after insertion: %d\n", i->n);

  sort_list(i->free_rrn, i->n);
  write_rrn_list_to_file(i);

  if (app.debug) {
    printf("List after insertion:\n");
    for (int j = 0; j < i->n; j++)
      printf("%hu ", i->free_rrn[j]);
    puts("");
    printf("@RRN %d added and list sorted. New list:\n", rrn);
    for (int j = 0; j < i->n; j++)
      printf("%d ", i->free_rrn[j]);
    puts("");
  }
}

io_buf *alloc_io_buf(void) {
  io_buf *io = malloc(sizeof(io_buf));
  if (!io) {
    puts("!!Could not allocate IO_BUFFER");
    return NULL;
  }

  io->hr = malloc(sizeof(data_header_record));
  io->br = malloc(sizeof(index_header_record));
  if (!io->hr || !io->br) {
    puts("!!Could not allocate header records");
    free(io);
    return NULL;
  }

  io->hr->free_rrn_address = malloc(sizeof(char) * MAX_ADDRESS);
  io->br->free_rrn_address = malloc(sizeof(char) * MAX_ADDRESS);
  if (!io->hr->free_rrn_address || !io->br->free_rrn_address) {
    puts("!!Could not allocate free_rrn_address");
    free(io->hr);
    free(io->br);
    free(io);
    return NULL;
  }

  if (app.debug) {
    puts("@Allocated IO_BUFFER");
  }
  return io;
}

void d_insert(io_buf *io, data_record *d, free_rrn_list *ld, u16 rrn) {
  if (!io || !d || !ld) {
    puts("!!Error: NULL parameters on d_insert");
  }
  if (rrn == (u16)-1)
    rrn = get_free_rrn(ld);

  write_data_record(io, d, rrn);
}

void load_data_header(io_buf *io) {
  if (!io || !io->fp) {
    puts("!!Invalid IO buffer or file pointer");
    return;
  }

  data_header_record temp_hr = {0};

  fseek(io->fp, 0, SEEK_SET);
  if (fread(&temp_hr.header_size, sizeof(u16), 1, io->fp) != 1 ||
      fread(&temp_hr.record_size, sizeof(u16), 1, io->fp) != 1) {
    puts("!!Error while reading header record (fixed part)");
    return;
  }

  if (app.debug)
    printf("Header_size: %hu \t Record size: %hu\n", temp_hr.header_size,
           temp_hr.record_size);

  if (!io->hr) {
    io->hr = malloc(sizeof(data_header_record));
    if (!io->hr) {
      puts("!!Memory allocation error");
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

  io->hr->free_rrn_address = malloc(rrn_len + 1);
  if (!io->hr->free_rrn_address) {
    puts("!!Memory allocation error for free_rrn_address");
    return;
  }

  fseek(io->fp, sizeof(u16) * 2, SEEK_SET);
  if (fread(io->hr->free_rrn_address, rrn_len, 1, io->fp) != 1) {
    puts("!!Error while reading free_rrn_address");
    free(io->hr->free_rrn_address);
    io->hr->free_rrn_address = NULL;
    return;
  }

  io->hr->free_rrn_address[rrn_len] = '\0';

  if (app.debug) {
    printf("--> data_header: record_size: %hu size: %hu free_rrn_address: %s\n",
           io->hr->record_size, io->hr->header_size, io->hr->free_rrn_address);
  }
}

void print_data_record(data_record *hr) {
  puts("\n--------DATA RECORD--------");
  printf("Placa: %s\n", hr->placa);
  printf("Modelo: %s\n", hr->modelo);
  printf("Marca: %s\n", hr->marca);
  printf("Ano: %d\n", hr->ano);
  printf("Categoria: %s\n", hr->categoria);
  printf("Quilometragem: %d\n", hr->quilometragem);
  printf("Status: %s\n", hr->status);
  puts("---------------------------\n");
}

data_record *load_data_record(io_buf *io, u16 rrn) {
  if (!io || !io->fp) {
    puts("!!Invalid IO buffer or file pointer");
    return NULL;
  }

  data_record *hr = malloc(sizeof(data_record));

  if (!hr) {
    puts("!!Memory allocation failed for data record");
    return NULL;
  }

  int byte_offset = io->hr->header_size + (io->hr->record_size * rrn);
  if (app.debug)
    printf("Header size: %d, Record size: %d, RRN: %d, byte_offset: %d\n",
           io->hr->header_size, io->hr->record_size, rrn, byte_offset);

  if (fseek(io->fp, byte_offset, SEEK_SET) != 0) {
    puts("!!Error seeking to byte offset");
    free(hr);
    return NULL;
  }

  size_t t = fread(hr, sizeof(data_record), 1, io->fp);
  if (t != 1) {
    puts("!!Error while reading data record");
    free(hr);
    return NULL;
  }
  return hr;
}

void prepend_data_header(io_buf *io) {
  if (!io || !io->fp || !io->hr || !io->hr->free_rrn_address) {
    puts("!!Invalid input in prepend_data_header");
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
    puts("!!File is empty, no content to prepend");
    original_file_size = 0;
  }

  char *buffer = NULL;
  if (original_file_size > 0) {
    buffer = malloc(original_file_size);
    if (buffer == NULL) {
      puts("!!Memory allocation failed for file content");
      return;
    }

    fseek(io->fp, 0, SEEK_SET);
    if (fread(buffer, 1, original_file_size, io->fp) != original_file_size) {
      puts("!!Error reading original file content");
      free(buffer);
      return;
    }
  }

  fclose(io->fp);
  io->fp = fopen(io->address, "wb");
  if (io->fp == NULL) {
    puts("!!Error reopening file in write mode");
    if (buffer)
      free(buffer);
    return;
  }

  printf("free rrn address: %s\n", io->hr->free_rrn_address);
  fseek(io->fp, 0, SEEK_SET);
  if (fwrite(&io->hr->header_size, sizeof(u16), 1, io->fp) != 1 ||
      fwrite(&io->hr->record_size, sizeof(u16), 1, io->fp) != 1 ||
      fwrite(io->hr->free_rrn_address, free_rrn_len, 1, io->fp) != 1) {
    puts("!!Error writing header to file");
    if (buffer)
      free(buffer);
    return;
  }

  if (buffer && original_file_size > 0) {
    if (fwrite(buffer, 1, original_file_size, io->fp) != original_file_size) {
      puts("!!Error writing original content after header");
    }
    free(buffer);
  }

  if (app.debug)
    printf("@Successfully written header: %hu %hu %s\n", io->hr->record_size,
           io->hr->header_size, io->hr->free_rrn_address);

  fflush(io->fp);
}

void write_data_record(io_buf *io, data_record *d, u16 rrn) {
  if (!io || !io->fp || !d) {
    puts("!!Invalid input in write_data_record");
    return;
  }

  int byte_offset = io->hr->header_size + (io->hr->record_size * rrn);
  fseek(io->fp, byte_offset, SEEK_SET);
  size_t t = fwrite(d, sizeof(data_record), 1, io->fp);
  if (t != 1) {
    puts("!!Error while writing data record");
  }
}

void populate_header(data_header_record *hp, const char *file_name) {
  if (hp == NULL) {
    puts("!!Header pointer is NULL, cannot populate");
    return;
  }

  hp->record_size = sizeof(data_record);
  strcpy(hp->free_rrn_address, file_name);
  hp->free_rrn_address[strlen(file_name) + 1] = '\0';
  hp->header_size = strlen(file_name) + 1 + sizeof(u16) * 2;
}

void load_file(io_buf *io, char *file_name, const char *type) {
  if (!file_name) {
    puts("!!ERROR: no file");
    return;
  }

  if (io->fp != NULL) {
    if (app.debug)
      puts("--> buffer already filled\n--> closing logical link\n");
    if (fclose(io->fp) != 0) {
      puts("!!ERROR: failed to close file");
      return;
    }
    io->fp = NULL;
  }

  memcpy(io->address, file_name, strlen(file_name));
  io->address[strlen(file_name) + 1] = '\0';

  printf("@Loading file: %s\n", file_name);

  io->fp = fopen(io->address, "r+b");
  if (io->fp == NULL) {
    puts("!!Error opening file");
    return;
  }

  if (strcmp(type, "index") == 0) {
    if (io->br == NULL) {
      io->br = malloc(sizeof(index_header_record));
      if (io->br == NULL) {
        puts("!!Memory allocation failed for index_header_record");
        return;
      }
    }
    load_index_header(io);
    if (!io->br || !io->hr)
      exit(-1);
  }

  if (strcmp(type, "data") == 0) {
    if (io->hr == NULL) {
      io->hr = malloc(sizeof(data_header_record));
      if (io->hr == NULL) {
        puts("!!Memory allocation failed for data_header_record");
        fclose(io->fp);
        return;
      }
    }
    load_data_header(io);
  }
  if (strcmp(type, "data") == 0 &&
      (io->hr->record_size == 0 || io->hr->header_size == 0)) {
    puts("!!Error: one or more inputs in data_header_record are 0");
    fclose(io->fp);
    return;
  }

  if (strcmp(type, "index") == 0 &&
      (io->br->page_size == 0 || io->br->header_size == 0)) {
    puts("!!Error: one or more inputs in index_header_record are 0");
    fclose(io->fp);
    return;
  }

  puts("@File loaded");
}

void create_data_file(io_buf *io, char *file_name) {
  if (!io || !file_name) {
    puts("!!Invalid IO buffer or file name");
    return;
  }

  strcpy(io->address, file_name);

  if (io->fp != NULL) {
    puts("!!File already open");
    return;
  }

  int prepend = 0;
  char list_name[MAX_ADDRESS];
  strcpy(list_name, file_name);
  char *dot = strrchr(list_name, '.');
  if (dot)
    strcpy(dot, ".hlp");

  if (app.debug)
    printf("@Loading File: %s\n", io->address);
  io->fp = fopen(io->address, "r+b");
  if (!io->fp) {
    if (app.debug)
      printf("!!Error opening file: %s. Creating it...\n", io->address);
    io->fp = fopen(io->address, "wb");
    if (!io->fp) {
      puts("!!Error creating file");
      return;
    }
    fclose(io->fp);
    io->fp = fopen(io->address, "r+b");
    prepend = 1;
    populate_header(io->hr, list_name);
  } else {
    if (io->hr == NULL) {
      io->hr = malloc(sizeof(data_header_record));
      if (io->hr == NULL) {
        puts("!!Memory allocation failed for data_header_record");
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
    if (app.debug)
      puts("@Prepending data header");
    populate_header(io->hr, list_name);
    prepend_data_header(io);
  } else {
    if (app.debug)
      puts("@Header already exists, no need to prepend");
  }

  if (app.debug) {
    puts("@Data file created successfully");
  }
}

void clear_io_buf(io_buf *io) {
  if (!io)
    return;

  if (io->fp) {
    fclose(io->fp);
    io->fp = NULL;
  }

  if (io->hr) {
    free(io->hr->free_rrn_address);
    free(io->hr);
    io->hr = NULL;
  }

  if (io->br) {
    free(io->br->free_rrn_address);
    free(io->br);
    io->br = NULL;
  }

  free(io);
  if (app.debug) {
    puts("@IO_BUFFER cleared");
  }
}
