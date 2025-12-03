#ifndef _APP
#define _APP

#include "modules/memory/mem.h"
#include "modules/log/log.h"
#include "modules/btree/btree.h"
#include "modules/utils/utils.h"
#include "modules/config/config.h"

typedef struct __App  {
  bool debug;
  io_buf *idx;
  io_buf *data;
  b_tree_buf *b;
  free_rrn_list *ld;
  memory *mem;
  log_level min_log_level;
  btree_config b_cfg;
} App;

void cli();

void init_app(void);

void clear_app();

int main(int argc, char **argv);

#endif
