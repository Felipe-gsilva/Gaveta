#include "config.h"
#include "../log/log.h"
#include "../../App.h"

extern App app;

bool read_btree_config() {
  const char *path = BTREE_CONFIG_PATH;
  FILE *fp = fopen(path, "r");
  char key[STRING_BUFFER_SIZE], value[STRING_BUFFER_SIZE];
  btree_config cfg;

  if (!fp) {
    g_error(FILE_OPEN_ERROR, "Could not open %s", path);
    return false;
  }

  // load cfg
  while(!feof(fp)) {
    fscanf(fp, "%s: %s\n", key, value);
    if (strcmp(key, "order") == 0) 
      cfg.order = atoi(value);
  };


  // handle default vals
  if (cfg.order <= 0 || cfg.order >= 100)
    cfg.order = 20;

  app.b_cfg = cfg;
  g_info("Btree config file loaded");
  return true;
}
