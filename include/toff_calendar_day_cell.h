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

void toff_calendar_day_cell_set_date(ToffCalendarDayCell *self, time_t date);

/*
 * Converts the day cell's inner `GDateTime` date to a `time_t` to be used with
 * C's `time.h` functions.
 *
 * @param `self` a pointer to a `ToffCalendarDayCell` instance.
 * @returns a `time_t` representing the day cell's date in epoch time.
 */
time_t toff_calendar_day_cell_get_date_as_time_t(ToffCalendarDayCell *self);

/*
 * Adds `DE_HOLIDAY` or `DE_EVENT` information to a `ToffCalendarDayCell` instance.
 *
 * @param `self` a pointer to a `ToffCalendarDayCell` instance.
 * @param `type` a `day_event_types` value representing the event type.
 * @param `name` a `char*` with the event's name.
 * @returns a `boolean` indicating the action's success.
 */
bool toff_calendar_day_cell_add_event_information(
    ToffCalendarDayCell *self,
    enum day_event_types type,
    char* name
);

/*
 * Adds a list of `vacation`s to the day cell and updates `event_type` to `DE_VACATION`.
 *
 * @param `self` a pointer to a `ToffCalendarDayCell` instance.
 * @param `vacations` a pointer to a `vacation` array.
 * @param `vacations_size` a `size_t` indicating the vacation array's allocated size.
 * @param `vacations_count` a `size_t` indicating how many valid elements are in the vacation array.
 * @returns a `boolean` indicating the action's success.
 */
bool toff_calendar_day_cell_add_vacations(
    ToffCalendarDayCell *self,
    vacation *vacations,
    size_t vacations_size,
    size_t vacations_count
);

GDateTime* toff_calendar_day_cell_get_date(ToffCalendarDayCell *self);
gboolean toff_calendar_day_cell_is_today(ToffCalendarDayCell *self);
void toff_calendar_day_cell_set_is_today(ToffCalendarDayCell *self, bool is_today);
gboolean toff_calendar_day_cell_in_month(ToffCalendarDayCell *self);
void toff_calendar_day_cell_set_in_month(ToffCalendarDayCell *self, bool in_month);
gboolean toff_calendar_day_cell_is_weekend(ToffCalendarDayCell *self);
void toff_calendar_day_cell_set_is_weekend(ToffCalendarDayCell *self, bool is_weekend);
enum day_event_types toff_calendar_day_cell_get_event_type(ToffCalendarDayCell *self);

G_END_DECLS
#endif
