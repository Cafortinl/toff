#ifndef VACATION_H
#define VACATION_H

#include <time.h>

#include "date_utilities.h"
#include "sqlite_database_administrator.h"

#define VACATION_TABLE_COLUMN_COUNT 6

extern table_field_node vacation_table_columns[VACATION_TABLE_COLUMN_COUNT];

extern table_definition vacation_table;

typedef struct {
    int id;
    int number_of_days;
    int employee_id;
    char* employee_name;
    date_range dates;
    time_t date_solicited;
} vacation;

/*
 * Specialized INSERT query that automatically calculates the `number_of_days` field.
 */
bool create_vacation(const sqlite_database_administrator *dba, const vacation *data, bool free_saturdays);
#endif
