#ifndef BRANCH_H
#define BRANCH_H

#include "sqlite_database_administrator.h"

#define BRANCH_TABLE_COLUMN_COUNT 2

column_constraints branch_id_constraint = {CC_PRIMARY_KEY, "ASC"};

table_field_node branch_table_columns[BRANCH_TABLE_COLUMN_COUNT] = {
    {"ID", NULL, SQLITE_DBA_INTEGER, (column_constraints*) (&branch_id_constraint)},
    {"Name", NULL, SQLITE_DBA_TEXT, NULL}
};

table_definition branch_table = {
    "Branch",
    (table_field_node*) (&branch_table_columns),
    BRANCH_TABLE_COLUMN_COUNT
};
#endif
