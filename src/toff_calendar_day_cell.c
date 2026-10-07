#include "toff_calendar_day_cell.h"
#include "glib-object.h"
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
        GArray *vacations;
    } events;
    GDateTime *date;
};

/*
 * Reduces the amount of boilerplate needed.
 *
 * For more info: https://docs.gtk.org/gobject/tutorial.html#boilerplate-code
 */
G_DEFINE_TYPE(ToffCalendarDayCell, toff_calendar_day_cell, GTK_TYPE_WIDGET)

typedef enum {
    SIG_DATE_SELECTED = 1,
    N_SIGNALS
} ToffCalendarDayCellSignal;

static guint obj_signals[N_SIGNALS] = {0, };

typedef enum {
    PROP_IS_TODAY = 1,
    PROP_IN_MONTH,
    PROP_IS_WEEKEND,
    PROP_EVENT_TYPE,
    PROP_EVENT_NAME,
    PROP_VACATIONS,
    PROP_DATE,
    N_PROPERTIES
} ToffCalendarDayCellProperty;

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

void handle_click_gesture(
    GtkGestureClick *gesture,
    int n_press,
    double x,
    double y,
    gpointer user_data
) {
    ToffCalendarDayCell *self = TOFF_CALENDAR_DAY_CELL(user_data);
    g_signal_emit(self, obj_signals[SIG_DATE_SELECTED], 0);
}

void handle_date_selected(GObject *object, gpointer user_data) {
    ToffCalendarDayCell *self = TOFF_CALENDAR_DAY_CELL(object);

    if (!self->date)
        return;

    g_print(
        "%d-%d-%d clicked!\n",
        g_date_time_get_year(self->date),
        g_date_time_get_month(self->date),
        g_date_time_get_day_of_month(self->date)
    );
}

void cleanup_events(ToffCalendarDayCell *self) {
    switch ((enum day_event_types) self->event_type) {
        case DE_HOLIDAY:
        case DE_EVENT:
            g_clear_pointer(&self->events.event_name, g_free);
            break;

        case DE_VACATION:
            g_clear_pointer(&self->events.vacations, g_array_unref);
            break;

        default:
            break;
    }
    
    self->event_type = DE_NONE;
}

void update_day_label(ToffCalendarDayCell *self) {
    char day_str[3];
    snprintf(day_str, 3, "%d", (int) g_date_time_get_day_of_month(self->date));
    gtk_label_set_text(GTK_LABEL(self->day_label), day_str);
}

void update_events_box(ToffCalendarDayCell *self) {
    GtkWidget *label;
    switch (self->event_type) {
        case DE_HOLIDAY:
        case DE_EVENT:
            label = gtk_label_new(self->events.event_name);
            gtk_label_set_wrap(GTK_LABEL(label), TRUE);

            gtk_box_append(GTK_BOX(self->event_box), label);
            break;
        case DE_VACATION:
            for (size_t i = 0; i < (size_t) self->events.vacations->len; ++i) {
                label = gtk_label_new(
                    i < EVENT_BOX_MAX ?
                        g_array_index(self->events.vacations, vacation, i).employee_name
                    :
                        "..."
                );
                gtk_label_set_wrap(GTK_LABEL(label), TRUE);
                gtk_box_append(GTK_BOX(self->event_box), label);

                if (i >= EVENT_BOX_MAX)
                    break;
            }
            break;
        default:
            break;
    }
}

static void toff_calendar_day_cell_set_property (
    GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec
) {
    ToffCalendarDayCell *self = TOFF_CALENDAR_DAY_CELL(object);

    switch ((ToffCalendarDayCellProperty) property_id) {
        case PROP_IS_TODAY:
            self->is_today = g_value_get_boolean(value);
            break;

        case PROP_IN_MONTH:
            self->in_month = g_value_get_boolean(value);
            break;

        case PROP_IS_WEEKEND:
            self->is_weekend = g_value_get_boolean(value);
            break;

        case PROP_EVENT_TYPE:
            cleanup_events(self);
            self->event_type = (enum day_event_types) g_value_get_int(value);
            break;

        case PROP_EVENT_NAME:
            cleanup_events(self);
            self->events.event_name = g_value_dup_string(value);
            update_events_box(self);
            break;

        case PROP_VACATIONS:
            cleanup_events(self);
            self->events.vacations = g_value_dup_boxed(value);
            self->event_type = DE_VACATION;
            update_events_box(self);
            break;

        case PROP_DATE:
            g_clear_pointer(&self->date, g_date_time_unref);
            self->date = g_date_time_new_from_unix_utc(g_value_get_int64(value));
            update_day_label(self);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void toff_calendar_day_cell_get_property(
    GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec
) {
    ToffCalendarDayCell *self = TOFF_CALENDAR_DAY_CELL(object);

    switch ((ToffCalendarDayCellProperty) property_id) {
        case PROP_IS_TODAY:
            g_value_set_boolean(value, self->is_today);
            break;

        case PROP_IN_MONTH:
            g_value_set_boolean(value, self->in_month);
            break;

        case PROP_IS_WEEKEND:
            g_value_set_boolean(value, self->is_weekend);
            break;

        case PROP_EVENT_TYPE:
            g_value_set_int(value, self->event_type);
            break;

        case PROP_EVENT_NAME:
            g_value_set_string(value, self->events.event_name);
            break;

        case PROP_VACATIONS:
            g_value_set_boxed(value, self->events.vacations);
            break;

        case PROP_DATE:
            g_value_set_int64(value, g_date_time_to_unix(self->date));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

/*
 * Releases all references to other GObject members (reference cycles).
 *
 * For more info: https://docs.gtk.org/gobject/concepts.html#reference-counts-and-cycles
 */
static void toff_calendar_day_cell_dispose(GObject *object) {
    ToffCalendarDayCell *self = TOFF_CALENDAR_DAY_CELL(object);

    //Clearing the template children
    gtk_widget_dispose_template(GTK_WIDGET(self), TOFF_TYPE_CALENDAR_DAY_CELL);

    //Chaining up to parent's dispose implementation
    G_OBJECT_CLASS(toff_calendar_day_cell_parent_class)->dispose(object);
}

static void toff_calendar_day_cell_finalize(GObject *object) {
    ToffCalendarDayCell *self = TOFF_CALENDAR_DAY_CELL(object);

    cleanup_events(self);
    g_clear_pointer(&self->date, g_date_time_unref);

    //Chaining up to parent's finalize implementation
    G_OBJECT_CLASS(toff_calendar_day_cell_parent_class)->finalize(object);
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

    //Section: Method Overrides
    object_class->dispose = toff_calendar_day_cell_dispose;
    object_class->finalize = toff_calendar_day_cell_finalize;
    //EndSection: Method Overrides

    //Section: Object Properties
    object_class->set_property = toff_calendar_day_cell_set_property;
    object_class->get_property = toff_calendar_day_cell_get_property;

    obj_properties[PROP_IS_TODAY] =
        g_param_spec_boolean(
            "is_today",
            "Is today",
            "Specifies if the day cell represents the current day.",
            FALSE,
            G_PARAM_READWRITE
        );

    obj_properties[PROP_IN_MONTH] =
        g_param_spec_boolean(
            "in_month",
            "In month",
            "Specifies if the day cell is in the current month.",
            TRUE,
            G_PARAM_READWRITE
        );

    obj_properties[PROP_IS_WEEKEND] =
        g_param_spec_boolean(
            "is_weekend",
            "Is weekend",
            "Specifies if the day cell corresponds to a weekend day.",
            FALSE,
            G_PARAM_READWRITE
        );

    obj_properties[PROP_EVENT_TYPE] =
        g_param_spec_int(
            "event_type",
            "Event type",
            "Type of event that the specific day cell contains.",
            DE_NONE,
            DE_TYPES_COUNT,
            DE_NONE,
            G_PARAM_READWRITE
        );

    obj_properties[PROP_EVENT_NAME] =
        g_param_spec_string(
            "event_name",
            "Event name",
            "Name of the event in the day cell's day (when it applies).",
            NULL,
            G_PARAM_READWRITE
        );

    obj_properties[PROP_VACATIONS] =
        g_param_spec_boxed(
            "vacations",
            "Vacations",
            "A list of employee vacations occurring in the day cell's day.",
            G_TYPE_ARRAY, 
            G_PARAM_READWRITE
        );

    obj_properties[PROP_DATE] =
        g_param_spec_int64(
            "date",
            "Date",
            "The day cell's date.",
            0,
            G_MAXINT64,
            0,
            G_PARAM_READWRITE
        );

    g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
    //EndSection: Object Properties

    //Section: Object Signals
    obj_signals[SIG_DATE_SELECTED] = 
        g_signal_new(
            "date-selected",
            TOFF_TYPE_CALENDAR_DAY_CELL,
            G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
            0,
            NULL,
            NULL,
            NULL,
            G_TYPE_NONE,
            0
        );
    //EndSection: Object Signals

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

    //Section: Event controllers
    GtkGesture *click_gesture;
    click_gesture = gtk_gesture_click_new();
    g_signal_connect(click_gesture, "pressed", G_CALLBACK(handle_click_gesture), self);
    gtk_widget_add_controller(GTK_WIDGET(self), GTK_EVENT_CONTROLLER(click_gesture));
    //EndSection: Event controllers

    g_signal_connect(self, "date-selected", G_CALLBACK(handle_date_selected), NULL);
}

GtkWidget* toff_calendar_day_cell_new(void) {
    return GTK_WIDGET(g_object_new(TOFF_TYPE_CALENDAR_DAY_CELL, NULL));
}
