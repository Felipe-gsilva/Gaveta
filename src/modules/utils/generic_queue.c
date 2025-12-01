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

static generic_queue *create_generic_queue_entry(void *data, u32 data_size) {
  generic_queue *gq = g_alloc(sizeof(generic_queue));

  gq->data_size = data_size;
  gq->next = NULL;
  gq->last = NULL;
  if (data != NULL) {
    gq->data = g_alloc(data_size);
    memcpy(gq->data, data, data_size);
  }
  return gq;
}

bool init_generic_queue(generic_queue **gq, u32 data_size) {
  assert(!*gq && data_size > 0);

  if (!gqt.head) {
    init_gqt();
  }

  (*gq) = create_generic_queue_entry(NULL, data_size);
  // insert_gqt(gq);
  return true;
}

bool push_generic_queue(generic_queue **gq, void *data) {
  if (!gq || !*gq) {
    g_error(QUEUE_EMPTY, "Invalid pointer passed to push_queue");
    return false;
  }

  generic_queue *new = create_generic_queue_entry(data, (*gq)->data_size);

  if (!(*gq)) {
    *gq = new;
    return true;
  }

  generic_queue *aux = *gq;

  while (aux->next != NULL)
    aux = aux->next;

  // new->last = aux;
  aux->next = new;
  return true;
}

bool pop_generic_queue(generic_queue **gq, generic_queue *save_to) {
  assert(gq && *gq);

  if (save_to) {
    save_to->data_size = (*gq)->data_size;
    save_to->next = (*gq)->next;
    memcpy(save_to->data, (*gq), (*gq)->data_size);
  };

  generic_queue *aux = (*gq)->next;
  if (aux == NULL) {
    aux = create_generic_queue_entry(NULL, (*gq)->data_size);
    return true;
  }
    g_dealloc(*gq);
  *gq = aux;
  return true;
}

bool is_queue_empty(generic_queue **gq) {
  if (!gq || !*gq) {
    g_error(INVALID_POINTER, "Invalid pointer passed to queue emptiness check");
    return true;
  }

  if ((*gq)->data)
    return false;

  return true;
}

generic_queue *top_generic_queue(generic_queue **gq) {
  if (is_queue_empty(gq)) {
    g_error(QUEUE_EMPTY, "Trying to top a empty generic queue!");
    return NULL;
  }

  return (*gq);
}

void print_generic_queue(generic_queue **gq) {
  if (is_queue_empty(gq)) {
    return;
  }

  generic_queue *aux = (*gq);
  while (aux) {
    printf("%s\n", (char *)(*gq)->data);
    if (aux->next == NULL) {
      return;
    }
    aux = aux->next;
  }
}
