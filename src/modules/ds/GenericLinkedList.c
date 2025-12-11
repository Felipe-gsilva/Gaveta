#include "GenericLinkedList.h"
#include "../../modules/memory/mem.h"
#include "../../modules/log/log.h"

GenericLinkedList *create_ll_node(void *data, u32 data_size) {
  GenericLinkedList *ll = g_alloc(sizeof(GenericLinkedList));

  ll->data_size = data_size;
  ll->next = NULL;
  if (data != NULL) {
    ll->data = g_alloc(data_size);
    memcpy(ll->data, data, data_size);
    return ll;
  }

  ll->data = NULL;
  return ll;
}


bool init_ll(GenericLinkedList **ll, u32 data_size) { 
  assert(data_size > 0);
  (*ll) = create_ll_node(NULL, data_size);
  return true;
}

bool insert_ll(GenericLinkedList **ll, void *data) {
  if (!ll || !*ll) {
    g_error(LIST_ERROR, "LinkedList not initialized");
    return false;
  }

  GenericLinkedList *new_node = create_ll_node(data, (*ll)->data_size);
  if (!new_node || !new_node->data) {
    g_error(LIST_ERROR, "Could not create new generic LinkedList node");
    return false;
  }

  GenericLinkedList *sentinel = *ll;
  new_node->next = sentinel->next;
  sentinel->next = new_node;
  return true;
}

bool remove_ll(GenericLinkedList **ll, void *save_to) {
  if (is_ll_empty(ll)) return false;

  GenericLinkedList *sentinel = *ll;
  GenericLinkedList *node_to_remove = sentinel->next;

  if (save_to) {
    if (save_to == NULL) {
      save_to = g_alloc(sentinel->data_size);
    }
    memcpy(save_to, node_to_remove->data, node_to_remove->data_size);
  }

  sentinel->next = node_to_remove->next;

  if (node_to_remove->data) g_dealloc(node_to_remove->data);
  g_dealloc(node_to_remove);

  return true;
}

bool is_ll_empty(GenericLinkedList **ll) {
  if (*ll && (*ll)->next) return false;
  return true;
}

bool clear_ll(GenericLinkedList **ll) {
  if (!ll || !*ll) {
    g_warn(LIST_STATUS, "trying to dealocate a non allocated LinkedList!");
    return false;
  }

  GenericLinkedList *helper = (*ll);
  while (!is_ll_empty(ll)) remove_ll(ll, NULL);

  g_dealloc(helper);
  return true;
}

bool search_ll(GenericLinkedList **ll, void *data, bool (*cmp_fn)(void*, void*), GenericLinkedList **found_node) {
  if (!*ll || is_ll_empty(ll)) {
    g_error(LIST_ERROR, "Trying to search in a empty LinkedList");
    return false;
  }

  if (!cmp_fn) {
    g_error(LIST_ERROR, "No comparison function provided");
    return false;
  }

  GenericLinkedList *aux = (*ll)->next;

  if (!aux) {
    g_warn(LIST_STATUS, "LinkedList is empty");
    return false;
  }

  while (aux != NULL) {
    if (cmp_fn(aux->data, data)) {
      if (found_node) *found_node = aux;
      g_debug(LIST_STATUS, "Found LinkedList entry with same data");
      return true;
    }
    aux = aux->next;
  }
  
  g_warn(LIST_STATUS, "Could not find entry in LinkedList");
  return false;
}


void print_generic_linkedlist(GenericLinkedList **ll, print_callback_fn printer) {
  if (is_ll_empty(ll)) {
    printf("LinkedList is empty\n");
    return;
  }

  GenericLinkedList *aux = (*ll)->next;

  int i = 0;
  while (aux != NULL) {
    if (aux->data) {
      printf("LinkedList with data_size %d. Entry [%d]: ", (*ll)->data_size, i);
      printer(aux->data);
    }
    aux = aux->next;
    i++;
  }
}

bool export_ll_to_disk(GenericLinkedList **ll, char* path, write_fallback_fn fn) {
  if (is_ll_empty(ll)) {
    g_error(LIST_ERROR, "LinkedList is empty. Nothing to write!");
    return false;
  }

  FILE *fp = fopen(path, "w");
  if (!fp) {
    g_error(LIST_ERROR, "Could not write LinkedList to file");
    return false;
  }

  GenericLinkedList *aux = (*ll)->next;

  while(aux) {
    fn(fp, aux->data);
  }

  fclose(fp);
  return true;
}

bool read_ll_from_disk(GenericLinkedList **ll, char* path, read_fallback_fn fn) {
  if (!ll || !*ll) {
    g_error(LIST_ERROR, "Trying to read into a non initialized LinkedList");
    return false;
  }

  FILE *fp = fopen(path, "r");
  if (!fp) {
    g_error(LIST_ERROR, "Could not open file to read LinkedList");
    return false;
  }

  while (!feof(fp)) {
    void *data = g_alloc((*ll)->data_size);
    fn(fp, data);
    insert_ll(ll, data);
    g_dealloc(data);
  }

  fclose(fp);
  return true;
}

u32 get_ll_size(GenericLinkedList **ll) {
  if (!ll || !*ll) {
    g_error(LIST_ERROR, "Trying to get size of a non initialized LinkedList");
    return 0;
  }

  GenericLinkedList *aux = (*ll)->next;
  u32 count = 0;

  while (aux) {
    count++;
    aux = aux->next;
  }

  return count;
}
