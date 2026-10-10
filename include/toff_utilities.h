#ifndef TOFF_UTILITIES_H
#define TOFF_UTILITIES_H

#include <stdio.h>
#include <string.h>

#include "branch.h"
#include "calendar_event.h"
#include "date_utilities.h"
#include "department.h"
#include "employee.h"
#include "holiday.h"
#include "position.h"
#include "sqlite_database_administrator.h"
#include "vacation.h"

enum day_event_types {
    DE_NONE,
    DE_HOLIDAY,
    DE_EVENT,
    DE_VACATION,
    DE_TYPES_COUNT
};

// Section: Data Containers
/**
 * Stores `query_result` information in a specific data type.
 */
typedef struct {
    size_t size;
    void* data;
} result_information;

void result_information_free(result_information *results, void (*extra_processing)(result_information*));
//void day_event_information_free(day_event_information *day_information);
query_result* fetch_data_in_date_range(sqlite_database_administrator *dba, const table_definition table_information, date_range range);
result_information* get_holidays_in_date_range(sqlite_database_administrator *dba, date_range range);
void holiday_result_extra_processing(result_information *holidays);
result_information* get_events_in_date_range(sqlite_database_administrator *dba, date_range range);
void event_result_extra_processing(result_information *events);
result_information* get_vacations_in_date(sqlite_database_administrator *dba, time_t date);
result_information* get_vacations_in_date_range(sqlite_database_administrator *dba, date_range range);
void vacation_result_extra_processing(result_information *vacations);
#endif
