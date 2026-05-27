#ifndef DEPARTMENT_H
#define DEPARTMENT_H

#include "sqlite_database_administrator.h"

#define DEPARTMENT_TABLE_COLUMN_COUNT 2

column_constraints department_id_constraint = {CC_PRIMARY_KEY, "ASC"};

table_field_node department_table_columns[DEPARTMENT_TABLE_COLUMN_COUNT] = {
    {"ID", NULL, SQLITE_DBA_INTEGER, (column_constraints*) (&department_id_constraint)},
    {"Name", NULL, SQLITE_DBA_TEXT, NULL}
};

table_definition department_table = {
    "Department",
    (table_field_node*) (&department_table_columns),
    DEPARTMENT_TABLE_COLUMN_COUNT
};
#endif
