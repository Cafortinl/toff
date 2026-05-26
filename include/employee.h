#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include "sqlite_database_administrator.h"

#define EMPLOYEE_TABLE_COLUMN_COUNT 7

column_constraints id_constraint = {CC_PRIMARY_KEY, "ASC"};
column_constraints position_fk_constraint = {CC_FOREIGN_KEY | CC_ON_DELETE, "Position|ID"};
column_constraints department_fk_constraint = {CC_FOREIGN_KEY | CC_ON_DELETE, "Department|ID"};
column_constraints branch_fk_constraint = {CC_FOREIGN_KEY | CC_ON_DELETE, "Branch|ID"};

table_field_node columns[EMPLOYEE_TABLE_COLUMN_COUNT] = {
    {"ID", NULL, SQLITE_DBA_INTEGER, (column_constraints*) (&id_constraint)},
    {"Name", NULL, SQLITE_DBA_TEXT, NULL},
    {"WorkingSince", NULL, SQLITE_DBA_TEXT, NULL},
    {"PositionID", NULL, SQLITE_DBA_INTEGER, (column_constraints*) (&position_fk_constraint)},
    {"DepartmentID", NULL, SQLITE_DBA_INTEGER, (column_constraints*) (&department_fk_constraint)},
    {"BranchID", NULL, SQLITE_DBA_INTEGER, (column_constraints*) (&branch_fk_constraint)},
    {"IsActive", NULL, SQLITE_DBA_INTEGER, NULL},
};

table_definition employee_table = {
    "Employee",
    (table_field_node*) (&columns),
    EMPLOYEE_TABLE_COLUMN_COUNT
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
