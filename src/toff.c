#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "branch.h"
#include "date_utilities.h"
#include "department.h"
#include "employee.h"
#include "event.h"
#include "holiday.h"
#include "position.h"
#include "sqlite_database_administrator.h"
#include "vacation.h"

#define GRID_ROWS    6
#define GRID_COLUMNS 7

// Section: Data Containers
/**
 * Stores `query_result` information in a specific data type.
 */
typedef struct {
    size_t size;
    void* data;
} result_information;

/**
 * Stores all the information and events related to a specific day.
 */
typedef struct {
    bool is_event;
    bool is_weekend;
    bool is_holiday;
    vacation* vacations;
    size_t vacations_size;
    size_t vacations_count;
    time_t date;
} day_event_information;

/**
 *  Stores all the information to be shown in the calendar grid.
 */
typedef struct {
    date_range dates;
    day_event_information day_information[GRID_ROWS * GRID_COLUMNS];
} calendar_day_information;

void result_information_free(
    result_information *results,
    void (*extra_processing)(result_information*)
) {
    if (extra_processing) {
        extra_processing(results);
    }

    free(results->data);
    results->data = NULL;
    free(results);
    results = NULL;
}

void day_event_information_free(day_event_information *day_information) {
    for (size_t i = 0; i < day_information->vacations_size; ++i) {
        if (i < day_information->vacations_count)
            free(day_information->vacations[i].employee_name);
    }
}

void calendar_day_information_free(calendar_day_information *calendar_information) {
    for (size_t i = 0; i < GRID_ROWS * GRID_COLUMNS; ++i) {
        day_event_information_free(&(calendar_information->day_information[i]));
    }
}
// EndSection: Data Containers

query_result* fetch_data_in_date_range(
    sqlite_database_administrator *dba,
    const table_definition table_information,
    date_range range
) {
    if (!dba) {
        fprintf(stderr, "fetch_data_in_date_range - Error: no valid sqlite_database_administrator struct was provided.\n");
        return NULL;
    }

    if (!dba->database) {
        fprintf(stderr, "fetch_data_in_date_range - Error: no valid database connection.\n");
        return NULL;
    }

    if (!IS_DATE_RANGE_VALID(range)) {
        fprintf(stderr, "fetch_data_in_date_range - Error: end_date can not be before start_date.\n");
        return NULL;
    }

    char start_date_str[19], end_date_str[19];
    strftime(start_date_str, 19, "date('%F')", localtime(&range.start_date));
    strftime(end_date_str, 19, "date('%F')", localtime(&range.end_date));

    query_filter_node *date_filters = &(query_filter_node) {
        .left.node = &(query_filter_node) {
            .left.text = "start_date",
            .right.text = start_date_str,
            .node_types = SET_NODETYPES(QF_TEXT, QF_TEXT),
            .operation = QF_GTE
        },
        .right.node = &(query_filter_node) {
            .left.text = "end_date",
            .right.text = end_date_str,
            .node_types = SET_NODETYPES(QF_TEXT, QF_TEXT),
            .operation = QF_LTE
        },
        .node_types = SET_NODETYPES(QF_NODE, QF_NODE),
        .operation = QF_AND
    };

    query_result *query_results = sqlite_dba_fetch_items(
        dba,
        table_information.table_name,
        table_information.columns,
        table_information.column_count,
        date_filters
    );

    return query_results;
}

result_information* get_holidays_in_date_range(
    sqlite_database_administrator *dba, 
    date_range range
) {
    result_information* results = malloc(sizeof(result_information) * 1);
    results->size = 0;
    results->data = NULL;

    if (!dba) {
        fprintf(stderr, "get_holidays_in_date_range - Error: no valid sqlite_database_administrator struct was provided.\n");
        goto end;
    }

    if (!dba->database) {
        fprintf(stderr, "get_holidays_in_date_range - Error: no valid database connection.\n");
        goto end;
    }

    if (!IS_DATE_RANGE_VALID(range)) {
        fprintf(stderr, "get_holidays_in_date_range - Error: end_date can not be before start_date.\n");
        goto end;
    }

    query_result *query_results = fetch_data_in_date_range(
        dba,
        holiday_table,
        range
    );

    if (!query_results) {
        goto end;
    }

    holiday *holidays = malloc(sizeof(holiday) * (query_results->length / query_results->column_count));
    for (size_t i = 0; i < query_results->length; i += query_results->column_count) {
        /*
         *  Given that `results` was queried using `holiday_table_columns` in
         *  the `fields` param we know for a fact that:
         *  - results->columns[i + 0] = id,
         *  - results->columns[i + 1] = name,
         *  - results->columns[i + 2] = start_date,
         *  - results->columns[i + 3] = end_date,
         */
        int year, month, day;

        sscanf(query_results->columns[i + 2].value.as.text_value, "%d-%d-%d", &year, &month, &day);
        struct tm start_date_builder = {
            .tm_year = year - 1900,
            .tm_mon = month - 1,
            .tm_mday = day
        };

        sscanf(query_results->columns[i + 3].value.as.text_value, "%d-%d-%d", &year, &month, &day);
        struct tm end_date_builder = {
            .tm_year = year - 1900,
            .tm_mon = month - 1,
            .tm_mday = day
        };

        holidays[i / query_results->column_count] = (holiday) {
            .id = query_results->columns[i].value.as.integer_value,
            .name = strdup(query_results->columns[i + 1].value.as.text_value),
            .dates = {
                .start_date = mktime(&start_date_builder),
                .end_date = mktime(&end_date_builder)
            }
        };
    }
    results->size = query_results->length / query_results->column_count;
    results->data = holidays;

    sqlite_dba_query_result_free(query_results);

end:
    return results;
}

void holiday_result_extra_processing(result_information *holidays) {
    for (size_t i = 0; i < holidays->size; ++i) {
        free(((holiday*) holidays->data)[i].name);
    }
}

result_information* get_events_in_date_range(
    sqlite_database_administrator *dba, 
    date_range range
) {
    result_information* results = malloc(sizeof(result_information) * 1);
    results->size = 0;
    results->data = NULL;

    if (!dba) {
        fprintf(stderr, "get_events_in_date_range - Error: no valid sqlite_database_administrator struct was provided.\n");
        goto end;
    }

    if (!dba->database) {
        fprintf(stderr, "get_events_in_date_range - Error: no valid database connection.\n");
        goto end;
    }

    if (!IS_DATE_RANGE_VALID(range)) {
        fprintf(stderr, "get_events_in_date_range - Error: end_date can not be before start_date.\n");
        goto end;
    }

    query_result *query_results = fetch_data_in_date_range(
        dba,
        event_table,
        range
    );

    if (!query_results) {
        goto end;
    }

    event *events = malloc(sizeof(event) * (query_results->length / query_results->column_count));
    for (size_t i = 0; i < query_results->length; i += query_results->column_count) {
        /*
         *  Given that `results` was queried using `event_table_columns` in
         *  the `fields` param we know for a fact that:
         *  - results->columns[i + 0] = id,
         *  - results->columns[i + 1] = name,
         *  - results->columns[i + 2] = start_date,
         *  - results->columns[i + 3] = end_date,
         */
        int year, month, day;

        sscanf(query_results->columns[i + 2].value.as.text_value, "%d-%d-%d", &year, &month, &day);
        struct tm start_date_builder = {
            .tm_year = year - 1900,
            .tm_mon = month - 1,
            .tm_mday = day
        };

        sscanf(query_results->columns[i + 3].value.as.text_value, "%d-%d-%d", &year, &month, &day);
        struct tm end_date_builder = {
            .tm_year = year - 1900,
            .tm_mon = month - 1,
            .tm_mday = day
        };

        events[i / query_results->column_count] = (event) {
            .id = query_results->columns[i].value.as.integer_value,
            .name = strdup(query_results->columns[i + 1].value.as.text_value),
            .dates = {
                .start_date = mktime(&start_date_builder),
                .end_date = mktime(&end_date_builder)
            }
        };
    }
    results->size = query_results->length / query_results->column_count;
    results->data = events;

    sqlite_dba_query_result_free(query_results);

end:
    return results;
}

void event_result_extra_processing(result_information *events) {
    for (size_t i = 0; i < events->size; ++i) {
        free(((event*) events->data)[i].name);
    }
}

result_information* get_vacations_in_date_range(
    sqlite_database_administrator *dba, 
    date_range range
) {
    result_information* results = malloc(sizeof(result_information) * 1);
    results->size = 0;
    results->data = NULL;

    if (!dba) {
        fprintf(stderr, "get_vacations_in_date_range - Error: no valid sqlite_database_administrator struct was provided.\n");
        goto end;
    }

    if (!dba->database) {
        fprintf(stderr, "get_vacations_in_date_range - Error: no valid database connection.\n");
        goto end;
    }

    if (!IS_DATE_RANGE_VALID(range)) {
        fprintf(stderr, "get_vacations_in_date_range - Error: end_date can not be before start_date.\n");
        goto end;
    }

    table_field_node columns_to_fetch[VACATION_TABLE_COLUMN_COUNT + 1] = {
        {.column_name = "v.id"},
        {.column_name = "v.employee_id"},
        {.column_name = "v.start_date"},
        {.column_name = "v.end_date"},
        {.column_name = "v.number_of_days"},
        {.column_name = "v.date_solicited"},
        {.column_name = "e.name"}
    };

    table_definition vacations_employees = {
        .table_name = "vacations v JOIN employees e ON v.employee_id = e.id",
        .columns = columns_to_fetch,
        .column_count = VACATION_TABLE_COLUMN_COUNT + 1
    };

    query_result *query_results = fetch_data_in_date_range(
        dba,
        vacations_employees,
        range
    );

    if (!query_results) {
        goto end;
    }

    vacation *vacations = malloc(sizeof(vacation) * (query_results->length / query_results->column_count));
    for (size_t i = 0; i < query_results->length; i += query_results->column_count) {
        int year, month, day;

        sscanf(query_results->columns[i + 2].value.as.text_value, "%d-%d-%d", &year, &month, &day);
        struct tm start_date_builder = {
            .tm_year = year - 1900,
            .tm_mon = month - 1,
            .tm_mday = day
        };

        sscanf(query_results->columns[i + 3].value.as.text_value, "%d-%d-%d", &year, &month, &day);
        struct tm end_date_builder = {
            .tm_year = year - 1900,
            .tm_mon = month - 1,
            .tm_mday = day
        };

        sscanf(query_results->columns[i + 5].value.as.text_value, "%d-%d-%d", &year, &month, &day);
        struct tm date_solicited_builder = {
            .tm_year = year - 1900,
            .tm_mon = month - 1,
            .tm_mday = day
        };

        vacations[i / query_results->column_count] = (vacation) {
            .id = query_results->columns[i].value.as.integer_value,
            .number_of_days = query_results->columns[i + 4].value.as.integer_value,
            .employee_id = query_results->columns[i + 1].value.as.integer_value,
            .dates = {
                .start_date = mktime(&start_date_builder),
                .end_date = mktime(&end_date_builder)
            },
            .date_solicited = mktime(&date_solicited_builder),
            .employee_name = strdup(query_results->columns[i + 6].value.as.text_value)
        };
    }
    results->size = query_results->length / query_results->column_count;
    results->data = vacations;

    sqlite_dba_query_result_free(query_results);

end:
    return results;
}

void vacation_result_extra_processing(result_information *vacations) {
    for (size_t i = 0; i < vacations->size; ++i) {
        free(((vacation*) vacations->data)[i].employee_name);
    }
}

calendar_day_information* populate_calendar_information(sqlite_database_administrator *dba, int month, int year) {
    if (!dba) {
        fprintf(stderr, "populate_calendar_information - Error: no valid sqlite_database_administrator struct was provided.\n");
        return NULL;
    }

    if (!dba->database) {
        fprintf(stderr, "populate_calendar_information - Error: no valid database connection.\n");
        return NULL;
    }

    date_range dates = generate_calendar_date_range(month, year);
    char start_date_str[11];
    strftime(start_date_str, 11, "%F", localtime(&(dates.start_date)));
    char end_date_str[11];
    strftime(end_date_str, 11, "%F", localtime(&(dates.end_date)));

    printf("Date range: %s - %s\n", start_date_str, end_date_str);

    calendar_day_information *calendar_information = malloc(sizeof(calendar_day_information) * 1);
    calendar_information->dates = dates;

    result_information *holidays = get_holidays_in_date_range(dba, dates);
    result_information *events = get_events_in_date_range(dba, dates);
    result_information *vacations = get_vacations_in_date_range(dba, dates);

    struct tm date_iterator_builder = *(localtime(&dates.start_date));
    time_t date_iterator = mktime(&date_iterator_builder);
    int day_index = 0;
    while (date_iterator <= dates.end_date) {
        day_event_information current_day;
        
        current_day.date = date_iterator;
        current_day.vacations = NULL;
        current_day.vacations_size = 0;
        current_day.vacations_count = 0;
        current_day.is_event = current_day.is_holiday = current_day.is_weekend = false;

        current_day.is_weekend
            = localtime(&date_iterator)->tm_wday == 0 || localtime(&date_iterator)->tm_wday == 6;

        for (size_t i = 0; i < events->size; ++i) {
            event current_event = ((event*) events->data)[i];

            if (IS_DATE_WITHIN_RANGE(date_iterator, current_event.dates)) {
                current_day.is_event = true;
                break;
            }
        }

        for (size_t i = 0; i < holidays->size; ++i) {
            holiday current_holiday = ((holiday*) holidays->data)[i];

            if (IS_DATE_WITHIN_RANGE(date_iterator, current_holiday.dates)) {
                current_day.is_holiday = true;
                break;
            }
        }

        if (current_day.is_weekend || current_day.is_event || current_day.is_holiday)
            goto end;

        current_day.vacations = malloc(vacations->size * sizeof(vacation));
        current_day.vacations_size = vacations->size;
        for (size_t i = 0; i < vacations->size; ++i) {
            vacation current_vacation = ((vacation*) vacations->data)[i];

            if (!IS_DATE_WITHIN_RANGE(date_iterator, current_vacation.dates))
                continue;

            memcpy(&(current_day.vacations[current_day.vacations_count]), &current_vacation, sizeof(vacation) * 1);
            current_day.vacations[current_day.vacations_count].employee_name = strdup(current_vacation.employee_name);
            ++current_day.vacations_count;
        }

        if (!current_day.vacations_count) {
           free(current_day.vacations);
           current_day.vacations = NULL;
           current_day.vacations_size = 0;
           current_day.vacations_count = 0;
        }

end:
        calendar_information->day_information[day_index] = current_day;

        date_iterator_builder.tm_mday += 1;
        date_iterator = mktime(&date_iterator_builder);
        ++day_index;
    }

    result_information_free(holidays, &holiday_result_extra_processing);
    result_information_free(events, &event_result_extra_processing);
    result_information_free(vacations, &vacation_result_extra_processing);

    return calendar_information;
}

int main(void) {
    sqlite_database_administrator *dba = sqlite_dba_connect_to_db("./test_db.sqlite");
    if (!dba) {
        fprintf(stderr, "There was an error while trying to connect to the database\n");
        return 1;
    }

    calendar_day_information *calendar_information = populate_calendar_information(dba, 8, 2026);

    for (size_t i = 0; i < GRID_ROWS * GRID_COLUMNS; ++i) {
        day_event_information day_information = calendar_information->day_information[i];

        char date_str[11];
        strftime(date_str, 11, "%F", localtime(&day_information.date));

        printf("%zu. %s - is_event: %d, is_holiday: %d, is_weekend: %d\n", i, date_str, day_information.is_event, day_information.is_holiday, day_information.is_weekend);
        
        if (day_information.vacations_count == 0)
            continue;

        printf("\t");
        for (size_t j = 0; j < day_information.vacations_count; ++j) {
            vacation current_vacation = day_information.vacations[j];

            char start_date_str[11];
            strftime(start_date_str, 11, "%F", localtime(&(current_vacation.dates.start_date)));
            char end_date_str[11];
            strftime(end_date_str, 11, "%F", localtime(&(current_vacation.dates.end_date)));

            printf("%s (%s - %s)%s", current_vacation.employee_name, start_date_str, end_date_str, j != day_information.vacations_count - 1 ? ", " : "");
        }
        printf("\n");
    }

    calendar_day_information_free(calendar_information);

    bool successful_disconnection = sqlite_dba_disconnect_from_db(dba);
    if (!successful_disconnection) {
        fprintf(stderr, "There was an error while trying to disconnect from the database.\n");
        return 2;
    }
    return 0;
}
