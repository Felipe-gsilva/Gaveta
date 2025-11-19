#include "App.h"
#include "modules/log/log.h"
#include "modules/b-tree/b-tree-buf.h"
#include "modules/b-tree/free-rrn-list.h"
#include "modules/b-tree/io-buf.h"
#include "modules/utils/queue.h"
#include <assert.h>

App app;

bool set_envvar(const char *mode) {
  if (strcmp(mode, "Debug") == 0 || strcmp(mode, "DEBUG") == 0) {
    app.debug = true;
    return true;
  }

  app.debug = false;
  return false;
}

void get_id(int i, char *placa) {
  assert(strlen(placa) >= 0);

  if (i >= 3) {
    puts("Many tries");
    return;
  }
  puts("Digite a placa");
  scanf("%s", placa);
  if (strlen(placa) != 7) {
    puts("!!Wrong size mate");
    get_id(i + 1, placa);
  }
}

void cli() {
  int choice = -1;
  char placa[TAMANHO_PLACA], placa_b[TAMANHO_PLACA];
  page *p;
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
        get_id(0, placa);
        p = b_search(app.b, placa, &pos);
        if (p) {
          print_page(p);
          d = load_data_record(app.data, p->keys[pos].data_register_rrn);
          print_data_record(d);
          break;
        }
        puts("Page not found!");
        break;
      case 2:
        get_id(0, placa);
        get_id(0, placa_b);
        strcpy(kr.start_id, placa);
        strcpy(kr.end_id, placa_b);
        b_range_search(app.b, app.data, &kr);
        break;
      case 3:
        g_info("Inserindo Veiculo:\n");
        get_id(0, d->placa);
        g_info("Modelo:\n");
        scanf("%s", d->modelo);
        g_info("Marca:\n");
        scanf("%s", d->marca);
        g_info("Ano:\n");
        scanf("%d", &(d->ano));
        g_info("Categoria:\n");
        scanf("%s", d->categoria);
        g_info("Quilometragem:\n");
        scanf("%d", &(d->quilometragem));
        g_info("Status:\n");
        scanf("%s", d->status);

        u16 rrn = get_free_rrn(app.ld);
        if (rrn == 0 && ftell(app.data->fp) >=
          (app.data->hr->header_size + app.data->hr->record_size))
          rrn = get_free_rrn(app.ld);
        b_insert(app.b, app.data, d, rrn);
        d_insert(app.data, d, app.ld, rrn);
        break;
      case 4:
        get_id(0, placa);
        b_remove(app.b, app.data, placa);
        break;
      case 5:

        if (app.debug)
          print_page(app.b->root);
        break;
      default:
        g_info("Invalid choice.\n");
        break;
    }
  }
}

void init_app(void) {
  app.idx = alloc_io_buf();
  app.data = alloc_io_buf();
  app.b = alloc_tree_buf();
  app.ld = alloc_ilist();
  if (app.idx && app.data) {
    puts("@Allocated APP_BUFFER");
    return;
  }

  puts("!!Error while allocating APP_BUFFER");
  exit(EXIT_FAILURE);
}

void clear_app() {
  if (!app.debug)
    puts("See you soon!!");
  if (app.idx) {
    clear_io_buf(app.idx);
    app.idx = NULL;
  }
  if (app.data) {
    clear_io_buf(app.data);
    app.data = NULL;
  }
  clear_all_pages();
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

  strcpy(index_file, "public/btree-");
  converted_char = ORDER + '0';
  index_file[strlen(index_file)] = converted_char;
  strcat(index_file, ".idx");
  strcpy(data_file, "public/veiculos.dat");

  create_index_file(app.b->io, index_file);
  create_data_file(app.data, data_file);

  load_file(app.b->io, index_file, "index");
  load_file(app.data, data_file, "data");

  free(data_file);
  free(index_file);

  load_list(app.b->i, app.b->io->br->free_rrn_address);
  load_list(app.ld, app.data->hr->free_rrn_address);

  page *temp = load_page(app.b, app.b->io->br->root_rrn);
  app.b->root = temp;

  if (ftell(app.b->io->fp) <= app.b->io->br->header_size) {
    insert_list(app.b->i, 0);
    build_tree(app.b, app.data, n);

    if (app.debug)
      print_queue(app.b->q);

    insert_list(app.ld, n + 1);
    app.b->io->br->root_rrn = app.b->root->rrn;
    write_index_header(app.b->io);
  }

  cli();

  clear_app();
  return 0;
}
