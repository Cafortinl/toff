#include "toff_calendar_view.h"

#include <time.h>

#include "sqlite_database_administrator.h"
#include "toff_calendar_day_cell.h"
#include "toff_utilities.h"

#define GRID_ROWS 6
#define GRID_COLUMNS 7
#define VACATIONS_SIZE_INIT 4

struct _ToffCalendarView {
    GtkWidget parent;

    GtkWidget *month_selector;
    GtkWidget *year_selector;
    GtkWidget *calendar_grid;

    int month;
    int year;
};

/*
 * Reduces the amount of boilerplate needed.
 *
 * For more info: https://docs.gtk.org/gobject/tutorial.html#boilerplate-code
 */
G_DEFINE_TYPE(ToffCalendarView, toff_calendar_view, GTK_TYPE_BOX)

void populate_calendar_grid(ToffCalendarView *self) {
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(self->calendar_grid)))
        gtk_grid_remove(GTK_GRID(self->calendar_grid), child);

    sqlite_database_administrator *dba = sqlite_dba_connect_to_db("./test_db.sqlite");
    if (!dba) {
        g_printerr("populate_calendar_grid: There was an error while trying to connect to the database.\n");
        return;
    }

    //Generating date range
    date_range dates = generate_calendar_date_range(self->month, self->year);
    char start_date_str[11];
    strftime(start_date_str, 11, "%F", localtime(&(dates.start_date)));
    char end_date_str[11];
    strftime(end_date_str, 11, "%F", localtime(&(dates.end_date)));
    g_print("Date range: %s - %s\n", start_date_str, end_date_str);

    //Querying calendar information in date range
    result_information *holidays = get_holidays_in_date_range(dba, dates);
    result_information *events = get_events_in_date_range(dba, dates);

    struct tm date_iterator_builder = *(localtime(&dates.start_date));
    time_t date_iterator = mktime(&date_iterator_builder);
    int day_index = 0;
    while (date_iterator <= dates.end_date) {
        GtkWidget *current_day = toff_calendar_day_cell_new();

        toff_calendar_day_cell_set_date(TOFF_CALENDAR_DAY_CELL(current_day), date_iterator);
        toff_calendar_day_cell_set_is_weekend(
            TOFF_CALENDAR_DAY_CELL(current_day),
            localtime(&date_iterator)->tm_wday == 0 || localtime(&date_iterator)->tm_wday == 6
        );

        for (size_t i = 0; i < events->size; ++i) {
            event current_event = ((event*) events->data)[i];

            if (IS_DATE_WITHIN_RANGE(date_iterator, current_event.dates)) {
                toff_calendar_day_cell_add_event_information(
                    TOFF_CALENDAR_DAY_CELL(current_day),
                    DE_EVENT,
                    strdup(current_event.name)
                );
                goto can_have_vacations_check;
            }
        }

        for (size_t i = 0; i < holidays->size; ++i) {
            holiday current_holiday = ((holiday*) holidays->data)[i];

            if (IS_DATE_WITHIN_RANGE(date_iterator, current_holiday.dates)) {
                toff_calendar_day_cell_add_event_information(
                    TOFF_CALENDAR_DAY_CELL(current_day),
                    DE_HOLIDAY,
                    strdup(current_holiday.name)
                );
                goto can_have_vacations_check;
            }
        }

can_have_vacations_check:
        if (
            toff_calendar_day_cell_is_weekend(TOFF_CALENDAR_DAY_CELL(current_day))
            || toff_calendar_day_cell_get_event_type(TOFF_CALENDAR_DAY_CELL(current_day)) == DE_EVENT
            || toff_calendar_day_cell_get_event_type(TOFF_CALENDAR_DAY_CELL(current_day)) == DE_HOLIDAY
        ) {
            goto end;
        }

        
        result_information *vacations = get_vacations_in_date(
            dba, 
            date_iterator
        );

        if (vacations) {
            toff_calendar_day_cell_add_vacations(
                TOFF_CALENDAR_DAY_CELL(current_day),
                vacations->data,
                vacations->size,
                vacations->size
            );
        }

        result_information_free(vacations, vacation_result_extra_processing);

end:
        gtk_grid_attach(
            GTK_GRID(self->calendar_grid),
            current_day,
            day_index % GRID_COLUMNS,
            day_index / GRID_COLUMNS,
            1,
            1
        );

        date_iterator_builder.tm_mday += 1;
        date_iterator = mktime(&date_iterator_builder);
        ++day_index;
    }

    result_information_free(holidays, &holiday_result_extra_processing);
    result_information_free(events, &event_result_extra_processing);

    bool successful_disconnection = sqlite_dba_disconnect_from_db(dba);
    if (!successful_disconnection) {
        g_printerr("populate_calendar_grid: There was an error while trying to disconnect to the database.\n");
        return;
    }
}

static void toff_calendar_view_dispose(GObject *gobject) {
    ToffCalendarView *self = TOFF_CALENDAR_VIEW(gobject);

    //Clearing the template children
    gtk_widget_dispose_template(GTK_WIDGET(self), TOFF_TYPE_CALENDAR_VIEW);

    //Chaining up to parent's dispose implementation
    G_OBJECT_CLASS(toff_calendar_view_parent_class)->dispose(gobject);
}

static void toff_calendar_view_constructed(GObject *gobject) {
    ToffCalendarView *self = TOFF_CALENDAR_VIEW(gobject);

    //Setting current time info
    time_t time_now = time(NULL);
    struct tm *current_time = localtime(&time_now);

    self->month = current_time->tm_mon + 1;
    self->year = current_time->tm_year + 1900;

    gtk_drop_down_set_selected(GTK_DROP_DOWN(self->month_selector), self->month - 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(self->year_selector), self->year);

    populate_calendar_grid(self);

    //Chaining up to parent's constructed implementation
    G_OBJECT_CLASS(toff_calendar_view_parent_class)->constructed(gobject);
}

static void toff_calendar_view_class_init(ToffCalendarViewClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = toff_calendar_view_dispose;
    object_class->constructed = toff_calendar_view_constructed;

    gtk_widget_class_set_template_from_resource(
        GTK_WIDGET_CLASS(klass),
        "/org/loveless/toff/toff_calendar_view.ui"
    );

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), ToffCalendarView, month_selector);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), ToffCalendarView, year_selector);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), ToffCalendarView, calendar_grid);
}

static void toff_calendar_view_init(ToffCalendarView *self) {
    gtk_widget_init_template(GTK_WIDGET(self));
}

GtkWidget* toff_calendar_view_new(void) {
    return GTK_WIDGET(g_object_new(TOFF_TYPE_CALENDAR_VIEW, NULL));
}
