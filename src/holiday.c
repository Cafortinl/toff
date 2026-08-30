#include "holiday.h"

table_field_node holiday_table_columns[HOLIDAY_TABLE_COLUMN_COUNT] = {
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

table_definition holiday_table = {
    .table_name = "holidays",
    .columns = (table_field_node*) (&holiday_table_columns),
    .column_count = HOLIDAY_TABLE_COLUMN_COUNT
};
