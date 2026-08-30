#include "branch.h"

table_field_node branch_table_columns[BRANCH_TABLE_COLUMN_COUNT] = {
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

table_definition branch_table = {
    .table_name = "branches",
    .columns = (table_field_node*) (&branch_table_columns),
    .column_count = BRANCH_TABLE_COLUMN_COUNT
};
