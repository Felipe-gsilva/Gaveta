#include "generic_queue.h"
#include "../log/log.h"
#include "../memory/mem.h"

generic_queue_table gqt;

// TODO
static void init_gqt() {
  gqt.capability = 10;
  gqt.size = 0;
  gqt.head = g_alloc(sizeof(u32) * gqt.capability);
  gqt.tail = g_alloc(sizeof(u32) * gqt.capability);
}

// TODO
static void insert_gqt(generic_queue **gq) {}

bool is_queue_empty(generic_queue **gq) {
  if (gq && *gq) return false;
  return true;
}

static generic_queue *create_generic_queue_entry(void *data, u32 data_size) {
  generic_queue *gq = g_alloc(sizeof(generic_queue));

  gq->data_size = data_size;
  gq->next = NULL;
  if (data != NULL) {
    gq->data = g_alloc(data_size);
    memcpy(gq->data, data, data_size);
    return gq;
  }

  gq->data = NULL;
  return gq;
}

bool init_generic_queue(generic_queue **gq, u32 data_size) {
  assert(data_size > 0);
  (*gq) = create_generic_queue_entry(NULL, data_size);
  return true;
}

bool push_generic_queue(generic_queue **gq, void *data) {
  if (!gq || !*gq) {
    g_error(QUEUE_ERROR, "Queue not initialized (Head is NULL)");
    return false;
  }

  generic_queue *new_node = create_generic_queue_entry(data, (*gq)->data_size);
  generic_queue *aux = *gq;

  while (aux->next != NULL) aux = aux->next;

  aux->next = new_node;
  return true;
}

bool pop_generic_queue(generic_queue **gq, generic_queue *save_to) {
  if (is_queue_empty(gq)) return false;

  generic_queue *sentinel = *gq;
  generic_queue *node_to_remove = sentinel->next;

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

generic_queue *top_generic_queue(generic_queue **gq) {
  if (is_queue_empty(gq)) {
    g_error(QUEUE_EMPTY, "Trying to top a empty generic queue!");
    return NULL;
  }

  return (*gq)->next;
}

void print_generic_queue(generic_queue **gq) {
  if (is_queue_empty(gq)) {
    printf("Queue is empty\n");
    return;
  }

  generic_queue *aux = (*gq)->next;
  
  int i = 0;
  while (aux != NULL) {
    if (aux->data) {
        printf("[%d]: %s\n", i, (char*)aux->data);
    }
    aux = aux->next;
    i++;
  }
}
