#include "config.h"
#include "../log/log.h"
#include "../../App.h"

extern App app;

// Example btree.conf file
// {
//   "btree_name": "DefaultTree",
//   "order": 32,
//   "index_file": "default.idx",
//   "data_file": "default.dat",
//   "schema": {
//     "id": "number",
//     "name": "string",
//     "value": "number"
//   },
// }

bool read_btree_config(const char *file_name, btree_config *cfg) {
  assert(cfg != NULL && file_name != NULL);
  FILE *fp = fopen(file_name, "r");
  char key[STRING_BUFFER_SIZE], value[STRING_BUFFER_SIZE];

  if (!fp) {
    g_error(FILE_OPEN_ERROR, "Could not open %s", file_name);
    return false;
  }

  // load cfg
  while(!feof(fp)) {
    fscanf(fp, "%s: %s\n", key, value);
    if (strcmp(key, "\"order\"") == 0) 
      cfg->order = atoi(value);
    else if (strcmp(key, "\"index_file\"") == 0)
      sprintf(cfg->index_file, "%s", value + 1);
    else if (strcmp(key, "\"data_file\"") == 0)
      sprintf(cfg->data_file, "%s", value + 1); 
    else if (strcmp(key, "\"schema_size\"") == 0)
      cfg->schema_size = atoi(value);
    else if (strcmp(key, "\"schema\"") == 0)
      // not usable yet
      cfg->schema = NULL;
    else if (strcmp(key, "\"btree_name\"") == 0) 
      sprintf(cfg->name, "%s", value + 1);
  };

  if (fclose(fp) != 0) {
    g_error(FILE_CLOSE_ERROR, "Could not close %s", file_name);
    return false;
  }

  if (cfg->order <= 0 || cfg->order >= 100) cfg->order = 20;
  
  if (cfg->schema_size == 0) cfg->schema_size = sizeof(key);

  if (strlen(cfg->name) <= 0) return false;

  sprintf(cfg->index_file, "assets/public/%s.idx", cfg->name);
  sprintf(cfg->data_file, "assets/public/%s.dat", cfg->name);

  g_info("Btree config file loaded");
  return true;
}
