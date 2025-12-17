#ifndef _SCHEMA_H
#define _SCHEMA_H

#include "../ds/HashTable.h"

typedef enum {
    G_INT = 0,
    G_UINT = 1,
    G_FLOAT = 2,
    G_DOUBLE = 3,
    G_CHAR = 7,
    G_STR = 8,
} SchemaTypes;

typedef struct {
    SchemaTypes type_id;
    bool is_nullable;
    int max_length;
} ColumnMetadata;

#endif
