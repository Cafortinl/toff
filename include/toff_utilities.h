#ifndef TOFF_UTILITIES_H
#define TOFF_UTILITIES_H

#include <stdio.h>
#include <string.h>

#include "branch.h"
#include "calendar_event.h"
#include "date_utilities.h"
#include "department.h"
#include "employee.h"
#include "position.h"
#include "sqlite_database_administrator.h"
#include "vacation.h"

// Section: Data Containers
/**
 * Stores `query_result` information in a specific data type.
 */
typedef struct {
    size_t size;
    void* data;
} result_information;

void result_information_free(result_information *results, void (*extra_processing)(result_information*));
query_result* fetch_data_in_date_range(sqlite_database_administrator *dba, const table_definition table_information, date_range range, const query_filter_node *extra_filters);
result_information* get_event_type_in_date_range(sqlite_database_administrator *dba, date_range range, enum day_event_types event_type);
void event_result_extra_processing(result_information *events);
result_information* get_vacations_in_date(sqlite_database_administrator *dba, time_t date);
result_information* get_vacations_in_date_range(sqlite_database_administrator *dba, date_range range);
void vacation_result_extra_processing(result_information *vacations);
#endif
