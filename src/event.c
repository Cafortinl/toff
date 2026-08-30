#include "event.h"

table_field_node event_table_columns[EVENT_TABLE_COLUMN_COUNT] = {
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
        .column_name = "name",
        .value = (column_information) {
            .type = SQLITE_DBA_TEXT
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
};

table_definition event_table = {
    .table_name = "events",
    .columns = (table_field_node*) (&event_table_columns),
    .column_count = EVENT_TABLE_COLUMN_COUNT
};
