#ifndef EVENT_H
#define EVENT_H

#include <time.h>

#include "date_utilities.h"
#include "sqlite_database_administrator.h"

#define EVENT_TABLE_COLUMN_COUNT 4

extern table_field_node event_table_columns[EVENT_TABLE_COLUMN_COUNT];

extern table_definition event_table;

typedef struct {
    int id;
    char* name;
    date_range dates;
} event;
#endif
