#include "department.h"

table_field_node department_table_columns[DEPARTMENT_TABLE_COLUMN_COUNT] = {
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

table_definition department_table = {
    .table_name = "departments",
    .columns = (table_field_node*) (&department_table_columns),
    .column_count = DEPARTMENT_TABLE_COLUMN_COUNT
};
