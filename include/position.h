#ifndef POSITION_H
#define POSITION_H

#include "sqlite_database_administrator.h"

#define POSITION_TABLE_COLUMN_COUNT 2

table_field_node position_table_columns[POSITION_TABLE_COLUMN_COUNT] = {
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
    }
};

table_definition position_table = {
    .table_name = "positions",
    .columns = (table_field_node*) (&position_table_columns),
    .column_count = POSITION_TABLE_COLUMN_COUNT
};
#endif
