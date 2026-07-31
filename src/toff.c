#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "branch.h"
#include "department.h"
#include "employee.h"
#include "holiday.h"
#include "position.h"
#include "sqlite_database_administrator.h"
#include "vacation.h"

typedef struct {
    size_t size;
    void* data;
} result_information;

/*
 *  Returns the last date in the given month.
 *
 *  @param `month` the (zero based) index of the month to check.
 *  @param `year` the year to check.
 *  @returns an `integer` representing the last date of the given month.
 */
int get_month_last_date(int month, int year) {
    struct tm date_builder = {
        .tm_year = year - 1900,
        .tm_mon = month + 1,
        .tm_mday = -1
    };
    mktime(&date_builder);

    return date_builder.tm_mday + 1;
}

result_information* get_holidays_in_date_range(
    sqlite_database_administrator *dba, 
    time_t start_date,
    time_t end_date
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

    if (difftime(mktime(localtime(&end_date)), mktime(localtime(&start_date))) < 0) {
        fprintf(stderr, "get_holidays_in_date_range - Error: end_date can not be before start_date.\n");
        goto end;
    }

    char start_date_str[19], end_date_str[19];
    strftime(start_date_str, 19, "date('%F')", localtime(&start_date));
    strftime(end_date_str, 19, "date('%F')", localtime(&end_date));

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
        holiday_table.table_name,
        holiday_table_columns,
        HOLIDAY_TABLE_COLUMN_COUNT,
        date_filters
    );
    if (!results) {
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
            .start_date = mktime(&start_date_builder),
            .end_date = mktime(&end_date_builder)
        };
    }
    results->size = query_results->length / query_results->column_count;
    results->data = holidays;

    sqlite_dba_query_result_free(query_results);

end:
    return results;
}

void holiday_result_information_free(result_information *holidays) {
    for (size_t i = 0; i < holidays->size; ++i) {
        free(((holiday*) holidays->data)[i].name);
    }
    free(holidays->data);
    holidays->data = NULL;
    free(holidays);
}

result_information* get_vacations_in_date_range(
    sqlite_database_administrator *dba, 
    time_t start_date,
    time_t end_date
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

    if (difftime(mktime(localtime(&end_date)), mktime(localtime(&start_date))) < 0) {
        fprintf(stderr, "get_vacations_in_date_range - Error: end_date can not be before start_date.\n");
        goto end;
    }

    char start_date_str[19], end_date_str[19];
    strftime(start_date_str, 19, "date('%F')", localtime(&start_date));
    strftime(end_date_str, 19, "date('%F')", localtime(&end_date));

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
        vacation_table.table_name,
        vacation_table_columns,
        VACATION_TABLE_COLUMN_COUNT,
        date_filters
    );
    if (!results) {
        goto end;
    }

    vacation *vacations = malloc(sizeof(vacation) * (query_results->length / query_results->column_count));
    for (size_t i = 0; i < query_results->length; i += query_results->column_count) {
        /*
         *  Given that `results` was queried using `vacation_table_columns` in
         *  the `fields` param we know for a fact that:
         *  - results->columns[i + 0] = id,
         *  - results->columns[i + 1] = employee_id,
         *  - results->columns[i + 2] = start_date,
         *  - results->columns[i + 3] = end_date,
         *  - results->columns[i + 4] = number_of_days,
         *  - results->columns[i + 5] = date_solicited
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
            .start_date = mktime(&start_date_builder),
            .end_date = mktime(&end_date_builder),
            .date_solicited = mktime(&date_solicited_builder)
        };
    }
    results->size = query_results->length / query_results->column_count;
    results->data = vacations;

    sqlite_dba_query_result_free(query_results);

end:
    return results;
}

void vacation_result_information_free(result_information *vacations) {
    free(vacations->data);
    vacations->data = NULL;
    free(vacations);
}

int main(void) {
    sqlite_database_administrator *dba = sqlite_dba_connect_to_db("./test_db.sqlite");
    if (!dba) {
        fprintf(stderr, "There was an error while trying to connect to the db.\n");
        return 1;
    }

    struct tm start_date_builder = {
        .tm_year = 2026 - 1900,
        .tm_mon = 1,
        .tm_mday = 1
    };
    time_t start_date = mktime(&start_date_builder);

    struct tm end_date_builder = {
        .tm_year = 2026 - 1900,
        .tm_mon = 2,
        .tm_mday = get_month_last_date(2, 2026)
    };
    time_t end_date = mktime(&end_date_builder);

    result_information *holidays = get_holidays_in_date_range(dba, start_date, end_date);
    for (size_t i = 0; i < holidays->size; ++i) {
        holiday current_holiday = ((holiday*) holidays->data)[i];
        printf("%zu. %s - (%s - %s)\n", i + 1, current_holiday.name, ctime(&current_holiday.start_date), ctime(&current_holiday.end_date));
    }
    holiday_result_information_free(holidays);
    
    bool successful_disconnection = sqlite_dba_disconnect_from_db(dba);
    if (!successful_disconnection) {
        fprintf(stderr, "There was an erro while trying to disconnect from the db.\n");
        return 2;
    }

    return 0;
}
