#ifndef VACATION_H
#define VACATION_H

#include "time.h"
#include "sqlite_database_administrator.h"

#define VACATION_TABLE_COLUMN_COUNT 6

table_field_node vacation_table_columns[VACATION_TABLE_COLUMN_COUNT] = {
    {
        .column_name = "id",
        .value = (column_information) {
            .type = SQLITE_DBA_INTEGER
        },
        .constraints = &(column_constraints) {
           .constraint_information = CC_PRIMARY_KEY,
           .value = "ASC"
        }
    },
    {
        .column_name = "employee_id",
        .value = (column_information) {
            .type = SQLITE_DBA_INTEGER 
        }, 
        .constraints = &(column_constraints) {
            .constraint_information = CC_FOREIGN_KEY | CC_ON_DELETE,
            .value = "employee|id"
        }
    },
    {
        .column_name = "start_date", 
        .value = (column_information) { 
            .type = SQLITE_DBA_TEXT 
        }
    },
    {
        .column_name = "end_date",
        .value = (column_information) {
            .type = SQLITE_DBA_TEXT
        }
    },
    {
        .column_name = "number_of_days",
        .value = (column_information) {
            .type = SQLITE_DBA_INTEGER
        }
    },
    {
        .column_name = "date_solicited",
        .value = (column_information) {
            .type = SQLITE_DBA_TEXT
        }
    },
};

table_definition vacation_table = {
    .table_name = "vacations",
    .columns = (table_field_node*) (&vacation_table_columns),
    .column_count = VACATION_TABLE_COLUMN_COUNT
};

typedef struct {
    int id;
    int number_of_days;
    int employee_id;
    time_t start_date;
    time_t end_date;
    time_t date_solicited;
} vacation;

/*
 * Specialized INSERT query that automatically calculates the `number_of_days` field.
 */
bool create_vacation(const sqlite_database_administrator *dba, const vacation *data, bool free_saturdays) {
    sqlite3_str *query_builder = sqlite3_str_new(dba->database);
    sqlite3_str_appendf(
        query_builder,
        "INSERT INTO vacations (employee_id, start_date, end_date, number_of_days, date_solicited)"
        " VALUES ("
            " %d,"
            " '%s',"
            " '%s',"
            " ("
                " WITH RECURSIVE dates(d) AS ("
                    " SELECT date('%s')"
                    " UNION ALL"
                    " SELECT date(d, '+1 day') FROM dates WHERE d < date('%s')"
            " )"
            " SELECT COUNT(*) FROM dates"
            " WHERE strftime('%%w', d) NOT IN (%s)"
                " AND NOT EXISTS (SELECT 1 FROM holidays h WHERE d BETWEEN h.start_date AND h.end_date)"
            " ),"
            " '%s'"
        ");",
        data->employee_id,
        data->start_date,
        data->end_date,
        data->start_date,
        data->end_date,
        free_saturdays ? (char*) "'0', '6'" : (char*) "'0'",
        data->date_solicited
    );
    char* query = sqlite3_str_finish(query_builder);

    bool statement_prepared = sqlite_dba_prepare_statement((sqlite_database_administrator*) dba, query);
    if (!statement_prepared)
        return false;

    sqlite3_free(query);

    query_result *results = sqlite_dba_execute_statement((sqlite_database_administrator*) dba);
    if (!results)
        return false;

    sqlite_dba_query_result_free(results);

    return true;
}
#endif
