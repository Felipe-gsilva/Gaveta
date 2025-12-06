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

void get_id(int i, char *first_key) {
  assert(strlen(first_key) > 0);

  if (i >= 3) {
    puts("Many tries");
    return;
  }
  puts("Digite a first_key");
  scanf("%s", first_key);
  if (strlen(first_key) != 7) {
    puts("!!Wrong size mate");
    get_id(i + 1, first_key);
  }
}

void init_app(void) {
  read_btree_config();
  init_mem(MB * 16);
  app.idx = alloc_io_buf();
  app.data = alloc_io_buf();
  app.b = alloc_tree_buf(app.b_cfg.order);
  app.ld = alloc_ilist();
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
  if (app.b) {
    clear_tree_buf(app.b);
    app.b = NULL;
  }
  if (app.ld) {
    clear_ilist(app.ld);
    app.ld = NULL;
  }
}

int main(int argc, char **argv) {
  init_app();
  int n = app.b_cfg.order;
  char *index_file = g_alloc(MAX_ADDRESS), *data_file = g_alloc(MAX_ADDRESS);

  sprintf(index_file, "assets/public/btree%c%d.idx", '-', n);
  strcpy(data_file, "assets/public/btree.dat");

  create_index_file(app.b->io, index_file);
  create_data_file(app.data, data_file);

  load_file(app.b->io, index_file, "index");
  load_file(app.data, data_file, "data");

  g_dealloc(data_file);
  g_dealloc(index_file);

  // load_list(app.b->i, app.b->io->br->free_rrn_address);
  // load_list(app.ld, app.data->hr->free_rrn_address);

  // btree_node *temp = load_btree_node(app.b, app.b->io->br->root_rrn);
  // app.b->root = temp;

  //if (ftell(app.b->io->fp) <= app.b->io->br->header_size) {
  //  insert_list(app.b->i, 0);
  //  build_tree(app.b, app.data, n);

  //  if (app.debug)
  //    print_queue(app.b->q);

  //  insert_list(app.ld, n + 1);
  //  app.b->io->br->root_rrn = app.b->root->rrn;
  //  write_index_header(app.b->io);
  //}
  // heterogeneous data type
  GenericQueue *gq2 = NULL;
  init_gq(&gq2, sizeof(btree_node)); 
  btree_node g = {.child_num = 0, .children= NULL, .keys = NULL, .leaf = 0, .next_leaf = 0, .rrn = 0};
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
