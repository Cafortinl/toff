#ifndef POSITION_H
#define POSITION_H

#include "sqlite_database_administrator.h"

#define POSITION_TABLE_COLUMN_COUNT 2

column_constraints position_id_constraint = {CC_PRIMARY_KEY, "ASC"};

table_field_node position_table_columns[POSITION_TABLE_COLUMN_COUNT] = {
    {"ID", NULL, SQLITE_DBA_INTEGER, (column_constraints*) (&position_id_constraint)},
    {"Name", NULL, SQLITE_DBA_TEXT, NULL}
};

table_definition position_table = {
    "Position",
    (table_field_node*) (&position_table_columns),
    POSITION_TABLE_COLUMN_COUNT
};
#endif
