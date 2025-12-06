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

bool remove_ll(GenericLinkedList **ll, GenericLinkedList *save_to) {
  if (is_ll_empty(ll)) return false;

  GenericLinkedList *sentinel = *ll;
  GenericLinkedList *node_to_remove = sentinel->next;

  if (save_to) {
    if (save_to->data == NULL) {
      save_to->data = g_alloc(sentinel->data_size);
    }
    memcpy(save_to->data, node_to_remove->data, node_to_remove->data_size);
    save_to->data_size = node_to_remove->data_size;
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


void print_generic_LinkedList(GenericLinkedList **ll, print_callback_fn printer) {
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
