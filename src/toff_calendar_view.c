#include "toff_calendar_view.h"

#include <stdlib.h>
#include <time.h>

#include "glib-object.h"
#include "glib.h"
#include "glibconfig.h"
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
    while ((child = gtk_widget_get_first_child(self->calendar_grid))) {
        gtk_grid_remove(GTK_GRID(self->calendar_grid), child);
    }

    sqlite_database_administrator *dba = sqlite_dba_connect_to_db("./test_db.sqlite");
    if (!dba) {
        g_printerr("populate_calendar_grid: There was an error while trying to connect to the database.\n");
        return;
    }

    //Getting current date
    time_t time_now = time(0);
    struct tm today = *(localtime(&time_now));
    
    //Generating date range
    date_range dates = generate_calendar_date_range(self->month, self->year);
    char start_date_str[11];
    strftime(start_date_str, 11, "%F", localtime(&(dates.start_date)));
    char end_date_str[11];
    strftime(end_date_str, 11, "%F", localtime(&(dates.end_date)));

    //Querying calendar information in date range
    result_information *holidays = get_holidays_in_date_range(dba, dates);
    result_information *events = get_events_in_date_range(dba, dates);

    struct tm date_iterator_builder = *(localtime(&dates.start_date));
    time_t date_iterator = mktime(&date_iterator_builder);

    int day_index = 0;
    while (date_iterator <= dates.end_date) {
        gboolean is_weekend;
        gint event_type;
        GtkWidget *current_day = toff_calendar_day_cell_new();

        g_object_set(
            G_OBJECT(current_day),
            "date",
            date_iterator,
            "is_weekend",
            localtime(&date_iterator)->tm_wday == 0 || localtime(&date_iterator)->tm_wday == 6,
            "is_today",
            today.tm_year == date_iterator_builder.tm_year && today.tm_mon == date_iterator_builder.tm_mon && today.tm_mday == date_iterator_builder.tm_mday,
            "in_month",
            date_iterator_builder.tm_mon == (self->month - 1),
            NULL
        );

        for (size_t i = 0; i < events->size; ++i) {
            event current_event = ((event*) events->data)[i];

            if (IS_DATE_WITHIN_RANGE(date_iterator, current_event.dates)) {
                g_object_set(
                    G_OBJECT(current_day),
                    "event_type",
                    DE_EVENT,
                    "event_name",
                    current_event.name,
                    NULL
                );
                goto can_have_vacations_check;
            }
        }

        for (size_t i = 0; i < holidays->size; ++i) {
            holiday current_holiday = ((holiday*) holidays->data)[i];

            if (IS_DATE_WITHIN_RANGE(date_iterator, current_holiday.dates)) {
                g_object_set(
                    G_OBJECT(current_day),
                    "event_type",
                    DE_HOLIDAY,
                    "event_name",
                    current_holiday.name,
                    NULL
                );
                goto can_have_vacations_check;
            }
        }

can_have_vacations_check:
        g_object_get(
            G_OBJECT(current_day),
            "is_weekend", &is_weekend,
            "event_type", &event_type,
            NULL
        );

        if (is_weekend || event_type == DE_EVENT || event_type == DE_HOLIDAY)
            goto end;
        
        result_information *vacations_result = get_vacations_in_date(
            dba, 
            date_iterator
        );

        if (vacations_result) {
            GArray *vacations = g_array_new(FALSE, FALSE, sizeof(vacation));
            g_array_insert_vals(vacations, 0, vacations_result->data, vacations_result->size);

            //Creating the GValue to set the vacations property
            GValue vacations_value = G_VALUE_INIT;
            g_value_init(&vacations_value, G_TYPE_ARRAY);
            g_value_set_boxed(&vacations_value, vacations);

            g_object_set_property(G_OBJECT(current_day), "vacations", &vacations_value);

            g_value_unset(&vacations_value);
            g_array_unref(vacations);
        }

        result_information_free(vacations_result, vacation_result_extra_processing);

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

static void handle_month_change(GObject *self, GParamSpec *pspec, gpointer user_data) {
    ToffCalendarView *parent = TOFF_CALENDAR_VIEW(user_data);

    parent->month = gtk_drop_down_get_selected(GTK_DROP_DOWN(self)) + 1;
    
    populate_calendar_grid(parent);
}

static void handle_year_change(GObject *self, gpointer user_data) {
    ToffCalendarView *parent = TOFF_CALENDAR_VIEW(user_data);

    parent->year = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(self));

    populate_calendar_grid(parent);
}

static void toff_calendar_view_dispose(GObject *object) {
    ToffCalendarView *self = TOFF_CALENDAR_VIEW(object);

    //Clearing the template children
    gtk_widget_dispose_template(GTK_WIDGET(self), TOFF_TYPE_CALENDAR_VIEW);

    //Chaining up to parent's dispose implementation
    G_OBJECT_CLASS(toff_calendar_view_parent_class)->dispose(object);
}

static void toff_calendar_view_class_init(ToffCalendarViewClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = toff_calendar_view_dispose;

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

    //Setting current time info
    time_t time_now = time(NULL);
    struct tm *current_time = localtime(&time_now);

    self->month = current_time->tm_mon + 1;
    self->year = current_time->tm_year + 1900;

    gtk_drop_down_set_selected(GTK_DROP_DOWN(self->month_selector), self->month - 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(self->year_selector), self->year);

    g_signal_connect(self->month_selector, "notify::selected-item", G_CALLBACK(handle_month_change), self);
    g_signal_connect(self->year_selector, "value-changed", G_CALLBACK(handle_year_change), self);

    populate_calendar_grid(self);
}

GtkWidget* toff_calendar_view_new(void) {
    return GTK_WIDGET(g_object_new(TOFF_TYPE_CALENDAR_VIEW, NULL));
}
