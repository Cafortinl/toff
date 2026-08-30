#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include "sqlite_database_administrator.h"

#define EMPLOYEE_TABLE_COLUMN_COUNT 7

extern table_field_node employee_table_columns[EMPLOYEE_TABLE_COLUMN_COUNT];
extern table_definition employee_table;

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
