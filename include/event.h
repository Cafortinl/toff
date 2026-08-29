#ifndef EVENT_H
#define EVENT_H

#include <time.h>

#include "date_utilities.h"
#include "sqlite_database_administrator.h"

#define EVENT_TABLE_COLUMN_COUNT 4

table_field_node event_table_columns[EVENT_TABLE_COLUMN_COUNT] = {
    {
        .column_name = "id",
        .value = (column_information) {
            .type = SQLITE_DBA_INTEGER
        }, 
        .constraints = &(column_constraints) {
            .constraint_information = CC_PRIMARY_KEY, 
            .value = "ASC"
        }
    },
    {
        .column_name = "name",
        .value = (column_information) {
            .type = SQLITE_DBA_TEXT
        }
    },
    {
        .column_name = "start_date",
        .value = (column_information) {
            .type = SQLITE_DBA_TEXT
        }
    },
    {
        .column_name = "end_date",
        .value = (column_information) {
            .type = SQLITE_DBA_TEXT
        }
    },
};

table_definition event_table = {
    .table_name = "events",
    .columns = (table_field_node*) (&event_table_columns),
    .column_count = EVENT_TABLE_COLUMN_COUNT
};

typedef struct {
    int id;
    char* name;
    date_range dates;
} event;
#endif
