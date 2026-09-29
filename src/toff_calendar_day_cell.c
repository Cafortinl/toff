#include "toff_calendar_day_cell.h"
#include "glib.h"
#include "gtk/gtk.h"
#include <stdio.h>
#include <string.h>

#define EVENT_BOX_MAX 2

struct _ToffCalendarDayCell {
    GtkWidget parent;

    // Widget members
    GtkWidget *main_box;
    GtkWidget *day_label;
    GtkWidget *event_box;

    // Day Information members
    gboolean is_today;
    gboolean in_month;
    gboolean is_weekend;
    enum day_event_types event_type;
    union {
        char *event_name;
        struct {
            vacation* data;
            size_t size;
            size_t count;
        } vacations;
    } events;
    GDateTime *date;
};

/*
 * Reduces the amount of boilerplate needed.
 *
 * For more info: https://docs.gtk.org/gobject/tutorial.html#boilerplate-code
 */
G_DEFINE_TYPE(ToffCalendarDayCell, toff_calendar_day_cell, GTK_TYPE_WIDGET)

void toff_calendar_day_cell_update_events_box(ToffCalendarDayCell *self) {
    switch (self->event_type) {
        case DE_HOLIDAY:
        case DE_EVENT:
            gtk_box_append(GTK_BOX(self->event_box), gtk_label_new(self->events.event_name));
            break;
        case DE_VACATION:
            for (size_t i = 0; i < self->events.vacations.count; ++i) {
                if (i < EVENT_BOX_MAX) {
                    gtk_box_append(
                        GTK_BOX(self->event_box),
                        gtk_label_new(self->events.vacations.data[i].employee_name)
                    );
                } else {
                    gtk_box_append(
                        GTK_BOX(self->event_box),
                        gtk_label_new("...")
                    );
                    break;
                }
            }
            break;
        default:
            break;
    }
}

/*
 * Releases all references to other GObject members (reference cycles).
 *
 * For more info: https://docs.gtk.org/gobject/concepts.html#reference-counts-and-cycles
 */
static void toff_calendar_day_cell_dispose(GObject *gobject) {
    ToffCalendarDayCell *self = TOFF_CALENDAR_DAY_CELL(gobject);

    //Clearing the template children
    gtk_widget_dispose_template(GTK_WIDGET(self), TOFF_TYPE_CALENDAR_DAY_CELL);

    //Chaining up to parent's dispose implementation
    G_OBJECT_CLASS(toff_calendar_day_cell_parent_class)->dispose(gobject);
}

static void toff_calendar_day_cell_finalize(GObject *gobject) {
    ToffCalendarDayCell *self = TOFF_CALENDAR_DAY_CELL(gobject);

    switch (self->event_type) {
        case DE_HOLIDAY:
        case DE_EVENT:
            g_free(self->events.event_name);
            break;
        case DE_VACATION:
            for (size_t i = 0; i < self->events.vacations.size; ++i) {
                if (i < self->events.vacations.count)
                    g_free(self->events.vacations.data[i].employee_name);
            }
            g_free(self->events.vacations.data);
            break;
        default:
            break;
    }

    g_clear_pointer(&self->date, g_date_time_unref);

    //Chaining up to parent's finalize implementation
    G_OBJECT_CLASS(toff_calendar_day_cell_parent_class)->finalize(gobject);
}

/*
 * This is called if it's the first instantiation of such an object.
 *
 * It is tasked with overriding the object's class methods and set up
 * the class' own methods.
 *
 * For more info: https://docs.gtk.org/gobject/concepts.html#object-instantiation
 */
static void toff_calendar_day_cell_class_init(ToffCalendarDayCellClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    object_class->dispose = toff_calendar_day_cell_dispose;
    object_class->finalize = toff_calendar_day_cell_finalize;

    gtk_widget_class_set_template_from_resource(
        GTK_WIDGET_CLASS(klass),
        "/org/loveless/toff/toff_calendar_day_cell.ui"
    );
    gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BIN_LAYOUT);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), ToffCalendarDayCell, main_box);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), ToffCalendarDayCell, day_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(klass), ToffCalendarDayCell, event_box);
}

/*
 *  `ToffCalendarDayCell`'s constructor function
 */
static void toff_calendar_day_cell_init(ToffCalendarDayCell *self) {
    gtk_widget_init_template(GTK_WIDGET(self));
}

GtkWidget* toff_calendar_day_cell_new(void) {
    return GTK_WIDGET(g_object_new(TOFF_TYPE_CALENDAR_DAY_CELL, NULL));
}

void toff_calendar_day_cell_set_date(ToffCalendarDayCell *self, time_t date) {
    self->date = g_date_time_new_from_unix_utc((gint64) date);
    char day_str[3];
    snprintf(day_str, 3, "%d", (int) g_date_time_get_day_of_month(self->date));
    gtk_label_set_text(GTK_LABEL(self->day_label), day_str);
}

time_t toff_calendar_day_cell_get_date_as_time_t(ToffCalendarDayCell *self) {
    g_clear_pointer(&self->date, g_date_time_unref);
    return (time_t) g_date_time_to_unix(self->date);
}

bool toff_calendar_day_cell_add_event_information(
    ToffCalendarDayCell *self,
    enum day_event_types type,
    char* name
) {
    if (type != DE_HOLIDAY && type != DE_EVENT)
        return false;

    self->event_type = type;
    self->events.event_name = strdup(name);

    toff_calendar_day_cell_update_events_box(self);

    return true;
}

bool toff_calendar_day_cell_add_vacations(
    ToffCalendarDayCell *self,
    vacation *vacations,
    size_t vacations_size,
    size_t vacations_count
) {
    if (!vacations)
        return false;

    self->event_type = DE_VACATION;
    self->events.vacations.data = calloc(vacations_size, sizeof(vacation));
    self->events.vacations.size = vacations_size;
    self->events.vacations.count = vacations_count;

    for (size_t i = 0; i < vacations_count; ++i) {
        self->events.vacations.data[i] = vacations[i];
        self->events.vacations.data[i].employee_name = strdup(vacations[i].employee_name);
    }

    toff_calendar_day_cell_update_events_box(self);

    return true;
}

GDateTime* toff_calendar_day_cell_get_date(ToffCalendarDayCell *self) {
    return self->date;
}

gboolean toff_calendar_day_cell_is_today(ToffCalendarDayCell *self) {
    return self->is_today;
}

void toff_calendar_day_cell_set_is_today(ToffCalendarDayCell *self, bool is_today) {
    self->is_today = is_today ? TRUE : FALSE;
}

gboolean toff_calendar_day_cell_in_month(ToffCalendarDayCell *self) {
    return self->in_month;
}

void toff_calendar_day_cell_set_in_month(ToffCalendarDayCell *self, bool in_month) {
    self->in_month = in_month ? TRUE : FALSE;
}

gboolean toff_calendar_day_cell_is_weekend(ToffCalendarDayCell *self) {
    return self->is_weekend;
}

void toff_calendar_day_cell_set_is_weekend(ToffCalendarDayCell *self, bool is_weekend) {
    self->is_weekend = is_weekend ? TRUE : FALSE;
}

enum day_event_types toff_calendar_day_cell_get_event_type(ToffCalendarDayCell *self) {
    return self->event_type;
}
