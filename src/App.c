#include "App.h"

App app;
extern log_level min_log_level;
extern bool debug;

bool set_envvar(const char *mode) {
  if (strcmp(mode, "Debug") == 0 || strcmp(mode, "DEBUG") == 0) {
    debug = true;
    return true;
  }

  debug = false;
  return false;
}

void init_app(void) {
  init_mem(MB * 16);
  app.idx = alloc_io_buf();
  app.data = alloc_io_buf();
  min_log_level = INFO;

  if (app.idx && app.data)
    return;

  g_crit_error(APP_ERROR, "Error while allocating APP_BUFFER");
}

void clear_app() {
  if (app.idx) {
    clear_io_buf(app.idx);
    app.idx = NULL;
  }
  if (app.data) {
    clear_io_buf(app.data);
    app.data = NULL;
  }
  clear_all_btree_nodes();
}

int main(int argc, char **argv) {
  init_app();
  

  // heterogeneous data type
  GenericQueue *gq2 = NULL;
  init_gq(&gq2, sizeof(btree_node));
  btree_node g = {.child_num = 0,
                  .children = NULL,
                  .keys = NULL,
                  .leaf = 0,
                  .next_leaf = 0,
                  .rrn = 0};
  push_gq(&gq2, &g);
  print_gq(&gq2, btree_node);
  push_gq(&gq2, &g);
  search_gq(&gq2, &g, compare_btree_nodes, NULL);

  pop_gq(&gq2, NULL);
  print_gq(&gq2, btree_node);
  pop_gq(&gq2, NULL);
  print_gq(&gq2, btree_node);
  clear_gq(&gq2);

  clear_app();
  return 0;
}
