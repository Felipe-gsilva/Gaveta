#ifndef _SCHEMA_H
#define _SCHEMA_H

#include "../ds/HashTable.h"

typedef enum __schema_types {
    INT = 0,
    STR = 1,
} schema_types;

typedef struct {
    schema_types type_id;
    bool is_nullable;
    int max_length;
} ColumnMetadata;

#endif
