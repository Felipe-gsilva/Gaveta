#include "GenericQueue.h"
#include "../log/log.h"
#include "../memory/mem.h"

bool is_gq_empty(GenericQueue **gq) {
  if (*gq && (*gq)->next) return false;
  return true;
}

GenericQueue *create_generic_queue_entry(void *data, u32 data_size) {
  GenericQueue *gq = g_alloc(sizeof(GenericQueue));

  gq->data_size = data_size;
  gq->next = NULL;
  gq->tail = NULL;
  if (data != NULL) {
    gq->data = g_alloc(data_size);
    memcpy(gq->data, data, data_size);
    return gq;
  }

  gq->data = NULL;
  return gq;
}

bool init_gq(GenericQueue **gq, u32 data_size) {
  assert(data_size > 0);
  (*gq) = create_generic_queue_entry(NULL, data_size);
  (*gq)->tail = *gq;
  return true;
}

bool push_gq(GenericQueue **gq, void *data) {
  if (!gq || !*gq) {
    g_error(QUEUE_ERROR, "Queue not initialized");
    return false;
  }

  GenericQueue *new_node = create_generic_queue_entry(data, (*gq)->data_size);
  if (!new_node || !new_node->data) {
    g_error(QUEUE_ERROR, "Could not create new generic queue node");
    return false;
  }

  GenericQueue *sentinel = *gq;
  sentinel->tail->next = new_node;

  sentinel->tail = new_node;

  return true;
}

bool pop_gq(GenericQueue **gq, GenericQueue *save_to) {
  if (is_gq_empty(gq)) return false;

  GenericQueue *sentinel = *gq;
  GenericQueue *node_to_remove = sentinel->next;

  if (save_to) {
    if (save_to->data == NULL) {
      save_to->data = g_alloc(sentinel->data_size);
    }
    memcpy(save_to->data, node_to_remove->data, node_to_remove->data_size);
    save_to->data_size = node_to_remove->data_size;
  }

  sentinel->next = node_to_remove->next;
  if (sentinel->next == NULL) sentinel->tail = sentinel;

  if (node_to_remove->data) g_dealloc(node_to_remove->data);
  g_dealloc(node_to_remove);

  return true;
}

GenericQueue *top_gq(GenericQueue **gq) {
  if (is_gq_empty(gq)) {
    g_error(QUEUE_EMPTY, "Trying to top a empty generic queue!");
    return NULL;
  }

  return (*gq)->next;
}

void print_generic_queue(GenericQueue **gq, print_callback_fn printer){
  if (is_gq_empty(gq)) {
    printf("Queue is empty\n");
    return;
  }

  GenericQueue *aux = (*gq)->next;

  int i = 0;
  while (aux != NULL) {
    if (aux->data) {
      printf("Queue with data_size %d. Entry [%d]: ", (*gq)->data_size, i);
      printer(aux->data);
    }
    aux = aux->next;
    i++;
  }
}

bool clear_gq(GenericQueue **gq) {
  if (!gq || !*gq) {
    g_warn(QUEUE_STATUS, "trying to dealocate a non allocated queue!");
    return false;
  }

  GenericQueue *helper = (*gq);
  while (!is_gq_empty(gq)) pop_gq(gq, NULL);

  g_dealloc(helper);
  return true;
}

bool search_gq(GenericQueue **gq, void *data, bool (*cmp_fn)(void *, void *), GenericQueue **found_node){
  if (!*gq || is_gq_empty(gq)) {
    g_error(QUEUE_ERROR, "Trying to search in a empty queue");
    return false;
  }

  if (!cmp_fn) {
    g_error(QUEUE_ERROR, "No comparison function provided");
    return false;
  }

  GenericQueue *aux = (*gq)->next;

  if (!aux) {
    g_warn(QUEUE_STATUS, "Queue is empty");
    return false;
  }

  while (aux != NULL) {
    if (cmp_fn(aux->data, data)) {
      if (found_node) *found_node = aux;
      g_debug(QUEUE_STATUS, "Found queue entry with same data");
      return true;
    }
    aux = aux->next;
  }
  
  g_warn(QUEUE_STATUS, "Could not find entry in queue");
  return false;
}
