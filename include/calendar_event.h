#ifndef CALENDAR_EVENT_H
#define CALENDAR_EVENT_H

#include <time.h>

#include "date_utilities.h"
#include "sqlite_database_administrator.h"

#define CALENDAR_EVENT_TABLE_COLUMN_COUNT 5

extern table_field_node calendar_event_table_columns[CALENDAR_EVENT_TABLE_COLUMN_COUNT];

extern table_definition calendar_event_table;

enum day_event_types {
    DE_NONE,
    DE_HOLIDAY,
    DE_EVENT,
    DE_VACATION,
    DE_TYPES_COUNT
};

typedef struct {
    int id;
    char* name;
    enum day_event_types event_type;
    date_range dates;
} event;
#endif
