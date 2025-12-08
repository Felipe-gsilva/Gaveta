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
  app.btrees = darray_create(sizeof(BTree *), 4);
  min_log_level = INFO;
}

void clear_app() {
  for (u32 i = 0; i < app.btrees->size; i++) {
    BTree btree;
    darray_get(app.btrees, i, &btree);
    clear_btree(&btree);
  }
  darray_destroy(app.btrees);
  clear_mem();
}

void cli() {
  char command[STRING_BUFFER_SIZE];
  while (true) {
    printf("btree-db> ");
    if (!fgets(command, STRING_BUFFER_SIZE, stdin)) {
      printf("\n");
      break;
    }
    // remove newline character
    command[strcspn(command, "\n")] = 0;
    if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0) {
      break;
    } else if (strcmp(command, "list") == 0) {
      printf("Loaded BTrees:\n");
      for (u32 i = 0; i < app.btrees->size; i++) {
        BTree *btree;
        darray_get(app.btrees, i, &btree);
      }
    } else {
      printf("Unknown command: %s\n", command);
    }
  }
}

int main(int argc, char **argv) {
  init_app();
  char cmd[STRING_BUFFER_SIZE];
  sprintf(cmd, "ls assets/configs | grep .json > assets/configs/list.txt");
  system(cmd);
  FILE *fp = fopen("assets/configs/list.txt", "r");
  if (!fp) {
    g_error(FILE_OPEN_ERROR, "Could not open assets/configs/list.txt");
    return -1;
  }
  char config_file[STRING_BUFFER_SIZE];
  while (fgets(config_file, STRING_BUFFER_SIZE, fp)) {
    config_file[strcspn(config_file, "\n")] = 0;
    char full_path[STRING_BUFFER_SIZE];
    sprintf(full_path, "assets/configs/%s", config_file);
    BTree *btree = create_btree(full_path);
    if (btree) {
      darray_push(app.btrees, &btree);
      g_info("BTree loaded from config: %s", config_file);
    } else {
      g_error(DEFAULT_STATUS, "Failed to create BTree from config: %s", config_file);
    }
  }
  fclose(fp);
  cli();
  clear_app();
  return 0;
}
