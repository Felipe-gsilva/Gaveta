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

static char* end_str(char *str) {
  for (size_t i = 0; i < strlen(str); i++) {
    if (str[i] == '"' || str[i] == ':' || str[i] == '\n' || str[i] == ' ') {
      str[i] = '\0';
    }
  }
  return str;
}

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
    // read a json line
    fscanf(fp, " %[^:]: %[^,\n]", key, value);

    if (strstr(key, "order"))
      cfg->order = atoi(value);
    else if (strstr(key, "index_file"))
      sprintf(cfg->index_file, "%s", end_str(value + 1));
    else if (strstr(key, "data_file"))
      sprintf(cfg->data_file, "%s", end_str(value + 1)); 
    else if (strstr(key, "schema_size"))
      cfg->schema_size = atoi(value);
    else if (strstr(key, "schema_data"))
      // not usable yet
      cfg->schema = NULL;
    else if (strstr(key, "btree_name")) 
      sprintf(cfg->name, "%s", end_str(value + 1));
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

  g_info("Btree config file for %s loaded", cfg->name);
  return true;
}
