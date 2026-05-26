#ifndef DEPARTMENT_H
#define DEPARTMENT_H

#include "sqlite_database_administrator.h"

#define DEPARTMENT_TABLE_COLUMN_COUNT 2

column_constraints id_constraint = {CC_PRIMARY_KEY, "ASC"};

table_field_node columns[DEPARTMENT_TABLE_COLUMN_COUNT] = {
    {"ID", NULL, SQLITE_DBA_INTEGER, (column_constraints*) (&id_constraint)},
    {"Name", NULL, SQLITE_DBA_TEXT, NULL}
};

table_definition position_table = {
    "Department",
    (table_field_node*) (&columns),
    DEPARTMENT_TABLE_COLUMN_COUNT
};
#endif
