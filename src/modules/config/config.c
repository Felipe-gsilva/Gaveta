#include "config.h"
#include "../log/log.h"
#include "../../App.h"
#include "../schema/schema.h"

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

bool read_schema(FILE *fp, btree_config *cfg, char *key, char *value) {
  init_hash_table(&cfg->schema, sizeof(ColumnMetadata));
  while(!feof(fp) || strstr(key, "}")) {
    // read a line
    fscanf(fp, " %[^:]: %s", key, value);
    printf("Key: %s, Value: %s\n", key, value);
    ColumnMetadata *cm = g_alloc(sizeof(ColumnMetadata));
    // if value not in acceptable SchemaTypes, do not put and throw error
    // TODO
    if(!put_ht(&cfg->schema, key, cm)) {
      g_error(HASH_TABLE_ERROR, "Could not insert %s into schema config", key);
      return false;
    }
  }

  return true;
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
    // read a line
    fscanf(fp, " %[^:]: %s", key, value);
    printf("Key: %s, Value: %s\n", key, value);
    if (strcmp(key, "{") == 0 || strcmp(key, "}") == 0)
      continue;

    if (strstr(key, "order"))
      cfg->order = atoi(value);
    else if (strstr(key, "index_file"))
      sprintf(cfg->index_file, "%s", end_str(value + 1));
    else if (strstr(key, "data_file"))
      sprintf(cfg->data_file, "%s", end_str(value + 1)); 
    else if (strstr(key, "idx_free_rrn_address"))
      sprintf(cfg->idx_free_rrn_address, "%s", end_str(value + 1));
    else if (strstr(key, "data_free_rrn_address"))
      sprintf(cfg->data_free_rrn_address, "%s", end_str(value + 1));
    else if (strstr(key, "schema_size"))
      cfg->schema_size = atoi(value);
    else if (strstr(key, "schema"))
        read_schema(fp, cfg, key, value);
    else if (strstr(key, "btree_name")) 
      sprintf(cfg->name, "%s", end_str(value + 1));
  };

  if (fclose(fp) != 0) {
    g_error(FILE_CLOSE_ERROR, "Could not close %s", file_name);
    return false;
  }

  char cmd[STRING_BUFFER_SIZE];
  #ifdef WIN32
    sprintf(cmd, "mkdir assets\\public\\%s", cfg->name);
    system(cmd);
  #else
    sprintf(cmd, "mkdir -p assets/public/%s", cfg->name);
    system(cmd);
  #endif


  if (cfg->order <= 0 || cfg->order >= 100) cfg->order = 20;
  
  if (cfg->schema_size == 0) cfg->schema_size = sizeof(key);

  if (strlen(cfg->name) <= 0) return false;

  sprintf(cfg->index_file, "assets/public/%s/BTree.idx", cfg->name);
  sprintf(cfg->data_file, "assets/public/%s/BTree.dat", cfg->name);

  if (strlen(cfg->idx_free_rrn_address) == 0)
    sprintf(cfg->idx_free_rrn_address, "assets/public/%s/idx_free_rrn.addr", cfg->name);

  if (strlen(cfg->data_free_rrn_address) == 0)
    sprintf(cfg->data_free_rrn_address, "assets/public/%s/data_free_rrn.addr", cfg->name);

  g_info("Btree config file for %s loaded", cfg->name);
  return true;
}

bool write_btree_config(const char *file_name, btree_config *cfg){
  assert(cfg && file_name);
  FILE *fp = fopen(file_name, "a");

  fclose(fp);
  return true;
}
