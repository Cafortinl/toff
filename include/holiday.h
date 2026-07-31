#ifndef HOLIDAY_H
#define HOLIDAY_H

#include "time.h"
#include "sqlite_database_administrator.h"

#define HOLIDAY_TABLE_COLUMN_COUNT 4

table_field_node holiday_table_columns[HOLIDAY_TABLE_COLUMN_COUNT] = {
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

table_definition holiday_table = {
    .table_name = "holidays",
    .columns = (table_field_node*) (&holiday_table_columns),
    .column_count = HOLIDAY_TABLE_COLUMN_COUNT
};

typedef struct {
    int id;
    char* name;
    time_t start_date;
    time_t end_date;
} holiday;
#endif
