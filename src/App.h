#ifndef _APP
#define _APP

#include "defines.h"
#include "modules/memory/mem.h"
#include "modules/log/log.h"
#include "modules/b-tree/b_tree.h"

typedef struct __App  {
  io_buf *idx;
  io_buf *data;
  b_tree_buf *b;
  free_rrn_list *ld;
  bool debug;
  memory *mem;
  log_level min_log_level;
} App;

void cli();

void init_app(void);

void clear_app();

int main(int argc, char **argv);

#endif
