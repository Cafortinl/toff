#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include "sqlite_database_administrator.h"

#define EMPLOYEE_TABLE_COLUMN_COUNT 7

table_field_node employee_table_columns[EMPLOYEE_TABLE_COLUMN_COUNT] = {
    {
        .column_name = "id",
        .value = (column_information) {
            .type = SQLITE_DBA_INTEGER},
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
        .column_name = "working_since",
        .value = (column_information) {
            .type = SQLITE_DBA_TEXT
        }
    },
    {
        .column_name = "position_id",
        .value = (column_information) {
            .type = SQLITE_DBA_INTEGER
        },
        .constraints = &(column_constraints) {
            .constraint_information = CC_FOREIGN_KEY | CC_ON_DELETE,
            .value = "position|id"
        }
    },
    {
        .column_name = "department_id",
        .value = (column_information) {
            .type = SQLITE_DBA_INTEGER
        },
        .constraints = &(column_constraints) {
            .constraint_information = CC_FOREIGN_KEY | CC_ON_DELETE,
            .value = "department|id"
        }
    },
    {
        .column_name = "branch_id",
        .value = (column_information) {
            .type = SQLITE_DBA_INTEGER
        },
        .constraints = &(column_constraints) {
            .constraint_information = CC_FOREIGN_KEY | CC_ON_DELETE,
            .value = "branch|id"
        }
    },
    {
        .column_name = "is_active",
        .value = (column_information) {
            .type = SQLITE_DBA_INTEGER
        }
    },
};

table_definition employee_table = {
    .table_name = "employees",
    .columns = (table_field_node*) (&employee_table_columns),
    .column_count = EMPLOYEE_TABLE_COLUMN_COUNT
};

typedef struct {
    bool is_active;
    char* name;
    int branch_id;
    int department_id;
    int position_id;
    int id;
    int day_balance;
} employee;
#endif
