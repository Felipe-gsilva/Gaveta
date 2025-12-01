#include "App.h"
#include "modules/utils/generic_queue.h"

App app;

bool set_envvar(const char *mode) {
  if (strcmp(mode, "Debug") == 0 || strcmp(mode, "DEBUG") == 0) {
    app.debug = true;
    return true;
  }

  app.debug = false;
  return false;
}

void get_id(int i, char *first_key) {
  assert(strlen(first_key) >= 0);

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

void cli() {
  int choice = -1;
  char *first_key, *second_key; // search keys
  btree_node *p;
  data_record *d = malloc(sizeof(data_record));
  u16 pos;
  key_range kr;

  while (choice != 0) {
    g_info("Choose an option:\n");
    g_info("0. Exit\n");
    g_info("1. Search by id\n");
    g_info("2. Search by range:\n");
    g_info("3. Insert\n");
    g_info("4. Remove\n");

    if (app.debug)
      g_info("5. Print root -- app.debug\n");

    g_info("Enter your choice: ");
    scanf("%d", &choice);

    switch (choice) {
      case 0:
        return;
      case 1:
        get_id(0, first_key);
        p = b_search(app.b, first_key, &pos);
        if (p) {
          print_btree_node(p);
          d = load_data_record(app.data, p->keys[pos].data_register_rrn);
          print_data_record(d);
          break;
        }
        puts("Page not found!");
        break;
      case 2:
        get_id(0, first_key);
        get_id(0, second_key);
        strcpy(kr.start_id->id, first_key);
        strcpy(kr.end_id->id, second_key);
        b_range_search(app.b, app.data, &kr);
        break;
      case 3:
        // TODO create function to read data_record from user
        u16 rrn = get_free_rrn(app.ld);
        if (rrn == 0 && ftell(app.data->fp) >=
          (app.data->hr->header_size + app.data->hr->record_size))
          rrn = get_free_rrn(app.ld);
        b_insert(app.b, app.data, d, rrn);
        d_insert(app.data, d, app.ld, rrn);
        break;
      case 4:
        get_id(0, first_key);
        b_remove(app.b, app.data, first_key);
        break;
      case 5:

        if (app.debug)
          print_btree_node(app.b->root);
        break;
      default:
        g_info("Invalid choice.\n");
        break;
    }
  }
}

void init_app(void) {
  init_mem(MB * 16);
  app.idx = alloc_io_buf();
  app.data = alloc_io_buf();
  app.b = alloc_tree_buf();
  app.ld = alloc_ilist();
  app.min_log_level = INFO;
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
  int n = 99;
  char *index_file = malloc(MAX_ADDRESS);
  char *data_file = malloc(MAX_ADDRESS);
  char converted_char;

  init_app();

  // strcpy(index_file, "public/btree-");
  // converted_char = convert_int_to_char(n);
  // index_file[strlen(index_file)] = converted_char;
  // strcat(index_file, ".idx");
  // strcpy(data_file, "public/veiculos.dat");

  // create_index_file(app.b->io, index_file);
  // create_data_file(app.data, data_file);

  // load_file(app.b->io, index_file, "index");
  // load_file(app.data, data_file, "data");

  free(data_file);
  free(index_file);

  // load_list(app.b->i, app.b->io->br->free_rrn_address);
  // load_list(app.ld, app.data->hr->free_rrn_address);

  // btree_node *temp = load_btree_node(app.b, app.b->io->br->root_rrn);
  // app.b->root = temp;

  // if (ftell(app.b->io->fp) <= app.b->io->br->header_size) {
  //   insert_list(app.b->i, 0);
  //   build_tree(app.b, app.data, n);

  //   if (app.debug)
  //     print_queue(app.b->q);

  //   insert_list(app.ld, n + 1);
  //   app.b->io->br->root_rrn = app.b->root->rrn;
  //   write_index_header(app.b->io);
  // }

  // cli();
  generic_queue *gq = NULL;
  init_generic_queue(&gq, sizeof(int)); 

  int val1 = 6;
  int val2 = 8;

  push_generic_queue(&gq, &val1);
  push_generic_queue(&gq, &val2);

  print_generic_queue(&gq); 

  pop_generic_queue(&gq, NULL);
  print_generic_queue(&gq);

  pop_generic_queue(&gq, NULL);
  print_generic_queue(&gq);

  clear_app();
  return 0;
}
