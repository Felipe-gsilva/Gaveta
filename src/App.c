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
  app.btrees = darray_create(sizeof(BTree), 4);
  min_log_level = INFO;
}

void clear_app() {
  for (u32 i = 0; i < app.btrees->size; i++) {
    BTree *btree = darray_get_pointer(app.btrees, i);
    clear_btree(btree);
  }
  darray_destroy(app.btrees);
  clear_mem();
}

void cli() {
  char command[STRING_BUFFER_SIZE];
  while (true) {
    printf("gaveta-db> ");
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
        BTree *btree = darray_get_pointer(app.btrees, i);
        printf("  [%d] %s with root RRN: %d\n", i, btree->config.name,
               btree->root ? btree->root->rrn : -1);
      }
    } else if (strstr(command, "insert")) {
      if (strlen(command) < strlen("insert 0"))  {
        printf("Error: use command ");
        printf("insert [0 - %d] ...  - Insert 0 {data}\n", app.btrees->size - 1);
      }
      continue;
    } else if (strcmp(command, "clear") == 0) {
#ifdef _WIN32
      system("cls");
#else
      system("clear");
#endif
    } else if (strcmp(command, "help") == 0 || strcmp(command, "h") == 0 ||
               strcmp(command, "?") == 0) {
      printf("Available commands:\n");
      printf("  list           - List all loaded BTrees\n");
      printf("  help, h, ?     - Show this help message\n");
      printf("  clear          - Clear the console screen\n");
      printf("  exit, quit     - Exit the application\n");
      printf("  insert [0 - %d] ...  - Insert 0 {data}\n", app.btrees->size - 1);
    } else {
      printf("Unknown command: %s\n", command);
    }
  }
}

int main(int argc, char **argv) {
  init_app();
  char cmd[STRING_BUFFER_SIZE];
  sprintf(cmd, "ls assets/configs | grep .conf > assets/configs/list.txt");
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
      darray_push(app.btrees, btree);
      g_info("BTree loaded from config: %s", config_file);
    } else {
      g_error(DEFAULT_STATUS, "Failed to create BTree from config: %s",
              config_file);
    }
  }
  fclose(fp);
  cli();
  clear_app();
  return 0;
}
