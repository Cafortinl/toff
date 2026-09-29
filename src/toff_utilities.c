#include "toff_utilities.h"

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

// void day_event_information_free(day_event_information *day_information) {
//     if (day_information->events.is_event || day_information.events.is_holiday)
//         free(day_information.events.event_name);
//
//     for (size_t i = 0; i < day_information->events.vacations.count; ++i) {
//         free(day_information->events.vacations.data[i].employee_name);
//     }
//     free(day_information->events.vacations.data);
// }

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
        calendar_event_table,
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

result_information* get_vacations_in_date(
    sqlite_database_administrator *dba,
    time_t date
) {
    result_information* results = malloc(sizeof(result_information) * 1);
    results->size = 0;
    results->data = NULL;

    if (!dba) {
        fprintf(stderr, "get_vacations_in_date - Error: no valid sqlite_database_administrator struct was provided.\n");
        goto end;
    }

    if (!dba->database) {
        fprintf(stderr, "get_vacations_in_date - Error: no valid database connection.\n");
        goto end;
    }

    char date_str[13];
    strftime(date_str, 13, "'%F'", localtime(&date));

    table_field_node columns_to_fetch[VACATION_TABLE_COLUMN_COUNT + 1] = {
        {.column_name = "v.id"},
        {.column_name = "v.employee_id"},
        {.column_name = "v.start_date"},
        {.column_name = "v.end_date"},
        {.column_name = "v.number_of_days"},
        {.column_name = "v.date_solicited"},
        {.column_name = "e.name"}
    };

    query_filter_node date_filter = {
        .node_types = SET_NODETYPES(QF_TEXT, QF_NODE),
        .left.text = date_str,
        .operation = QF_BETWEEN,
        .right.node = &(query_filter_node) {
            .node_types = SET_NODETYPES(QF_TEXT, QF_TEXT),
            .left.text = "v.start_date",
            .operation = QF_AND,
            .right.text = "v.end_date"
        }
    };

    query_result *query_results = sqlite_dba_fetch_items(
        dba,
        "vacations v JOIN employees e ON v.employee_id = e.id",
        columns_to_fetch,
        VACATION_TABLE_COLUMN_COUNT + 1,
        &date_filter
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
