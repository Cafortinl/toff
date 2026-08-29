#ifndef HOLIDAY_H
#define HOLIDAY_H

#include <time.h>

#include "date_utilities.h"
#include "sqlite_database_administrator.h"

#define HOLIDAY_TABLE_COLUMN_COUNT 4

extern table_field_node holiday_table_columns[HOLIDAY_TABLE_COLUMN_COUNT];

extern table_definition holiday_table;

typedef struct {
    int id;
    char* name;
    date_range dates;
} holiday;
#endif
