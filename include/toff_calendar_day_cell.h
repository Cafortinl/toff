#ifndef TOFF_CALENDAR_DAY_CELL_H
#define TOFF_CALENDAR_DAY_CELL_H

#include <gtk/gtk.h>

#include "toff_utilities.h"

enum day_event_types {
    DE_NONE,
    DE_HOLIDAY,
    DE_EVENT,
    DE_VACATION,
    DE_TYPES_COUNT
};

G_BEGIN_DECLS

/*
 *  Type Declaration
 *
 *  For more info: https://docs.gtk.org/gobject/tutorial.html
 */
#define TOFF_TYPE_CALENDAR_DAY_CELL (toff_calendar_day_cell_get_type ())
G_DECLARE_FINAL_TYPE(ToffCalendarDayCell, toff_calendar_day_cell, TOFF, CALENDAR_DAY_CELL, GtkWidget)

GtkWidget* toff_calendar_day_cell_new(void);

G_END_DECLS
#endif
