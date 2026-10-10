#include "toff_day_detail_view.h"

#include "date_utilities.h"
#include "glib-object.h"
#include "gtk/gtk.h"
#include "toff_utilities.h"
#include <math.h>

struct _ToffDayDetailView {
    GtkWidget parent;

    GtkWidget *day_label;
    GtkWidget *event_type_tag;
    GtkWidget *event_name;
    GtkWidget *vacation_list;

    enum day_event_types event_type;
    union {
        char *event_name;
        GArray *vacations;
    } events;
    GDateTime *date;
};

G_DEFINE_TYPE(ToffDayDetailView, toff_day_detail_view, GTK_TYPE_WINDOW)

typedef enum {
    PROP_EVENT_TYPE = 1,
    PROP_EVENT_NAME,
    PROP_VACATIONS,
    PROP_DATE,
    N_PROPERTIES
} ToffDayDetailViewProperty;

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void update_event_information(ToffDayDetailView *self) {
    switch ((enum day_event_types) self->event_type) {
        case DE_EVENT:
            gtk_label_set_text(GTK_LABEL(self->event_type_tag), "Event");
            gtk_widget_add_css_class(GTK_WIDGET(self->event_type_tag), "event-type-tag");
            gtk_widget_remove_css_class(GTK_WIDGET(self->event_type_tag), "holiday-tag");
            gtk_widget_add_css_class(GTK_WIDGET(self->event_type_tag), "event-tag");
            break;

        case DE_HOLIDAY:
            gtk_label_set_text(GTK_LABEL(self->event_type_tag), "Holiday");
            gtk_widget_add_css_class(GTK_WIDGET(self->event_type_tag), "event-type-tag");
            gtk_widget_remove_css_class(GTK_WIDGET(self->event_type_tag), "event-tag");
            gtk_widget_add_css_class(GTK_WIDGET(self->event_type_tag), "holiday-tag");
            break;

        default:
            break;
    }

    if (!self->event_name)
        return;

    gtk_label_set_text(GTK_LABEL(self->event_name), self->events.event_name);
}

static GtkWidget* create_vacation_list_item(vacation *vacation_information, time_t current_date) {
    date_range vacation_dates = vacation_information->dates;
    int days_to_current
        = DIFFTIME_IN_DAYS(current_date, vacation_dates.start_date);
    int total_days
        = DIFFTIME_IN_DAYS(vacation_dates.end_date, vacation_dates.start_date);

    int estimated_elapsed_days
        = (int) round((double) (days_to_current * vacation_information->number_of_days) / total_days);

    GtkWidget *content_box;
    GtkWidget *name_label;
    GtkWidget *info_label;

    content_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    name_label = gtk_label_new(g_strdup(vacation_information->employee_name));
    info_label = gtk_label_new(g_strdup_printf("%d/%d days", estimated_elapsed_days, vacation_information->number_of_days));

    gtk_box_append(GTK_BOX(content_box), GTK_WIDGET(name_label));
    gtk_box_append(GTK_BOX(content_box), GTK_WIDGET(info_label));

    return GTK_WIDGET(content_box);
}

static void populate_vacations_list(ToffDayDetailView *self) {
    if (!self->events.vacations)
        return;

    for (size_t i = 0; i < self->events.vacations->len; ++i) {
        gtk_list_box_append(
            GTK_LIST_BOX(self->vacation_list),
            create_vacation_list_item(
                &g_array_index(self->events.vacations, vacation, i),
                (time_t) g_date_time_to_unix(self->date)
            )
        );
    }
}

static void cleanup_events(ToffDayDetailView *self, bool clear_event_type) {
    switch (self->event_type) {
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

    if (clear_event_type)
        self->event_type = DE_NONE;
}

static void toff_day_detail_view_set_property (
    GObject *object,
    guint property_id,
    const GValue *value,
    GParamSpec *pspec
) {
    ToffDayDetailView *self = TOFF_DAY_DETAIL_VIEW(object);

    switch ((ToffDayDetailViewProperty) property_id) {
        case PROP_EVENT_TYPE:
            cleanup_events(self, true);
            self->event_type = g_value_get_int(value);
            update_event_information(self);
            break;

        case PROP_EVENT_NAME:
            cleanup_events(self, false);
            self->events.event_name = g_value_dup_string(value);
            update_event_information(self);
            break;

        case PROP_VACATIONS:
            cleanup_events(self, true);
            self->events.vacations = g_value_dup_boxed(value);
            self->event_type = DE_VACATION;
            populate_vacations_list(self);
            break;

        case PROP_DATE:
            g_clear_pointer(&self->date, g_date_time_unref);
            self->date = g_date_time_new_from_unix_utc(g_value_get_int64(value));
            gtk_label_set_text(GTK_LABEL(self->day_label), g_date_time_format(self->date, "%A, %B %d, %Y"));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void toff_day_detail_view_get_property (
    GObject *object,
    guint property_id,
    GValue *value,
    GParamSpec *pspec
) {
    ToffDayDetailView *self = TOFF_DAY_DETAIL_VIEW(object);

    switch ((ToffDayDetailViewProperty) property_id) {
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

static void toff_day_detail_view_dispose(GObject *object) {
    ToffDayDetailView *self = TOFF_DAY_DETAIL_VIEW(object);

    gtk_widget_dispose_template(GTK_WIDGET(self), TOFF_TYPE_DAY_DETAIL_VIEW);

    G_OBJECT_CLASS(toff_day_detail_view_parent_class)->dispose(object);
}

static void toff_day_detail_view_finalize(GObject *object) {
    ToffDayDetailView *self = TOFF_DAY_DETAIL_VIEW(object);

    cleanup_events(self, true);
    g_clear_pointer(&self->date, g_date_time_unref);

    G_OBJECT_CLASS(toff_day_detail_view_parent_class)->finalize(object);
}

static void toff_day_detail_view_class_init(ToffDayDetailViewClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    object_class->dispose = toff_day_detail_view_dispose;
    object_class->finalize = toff_day_detail_view_finalize;

    object_class->set_property = toff_day_detail_view_set_property;
    object_class->get_property = toff_day_detail_view_get_property;

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
    
    gtk_widget_class_set_template_from_resource(
        widget_class,
        "/org/loveless/toff/toff_day_detail_view.ui"
    );

    gtk_widget_class_bind_template_child(widget_class, ToffDayDetailView, day_label);
    gtk_widget_class_bind_template_child(widget_class, ToffDayDetailView, event_type_tag);
    gtk_widget_class_bind_template_child(widget_class, ToffDayDetailView, event_name);
    gtk_widget_class_bind_template_child(widget_class, ToffDayDetailView, vacation_list);
}

static void toff_day_detail_view_init(ToffDayDetailView *self) {
    gtk_widget_init_template(GTK_WIDGET(self));
}

GtkWidget* toff_day_detail_view_new(GtkWindow *parent) {
    return GTK_WIDGET(g_object_new(TOFF_TYPE_DAY_DETAIL_VIEW, "tansient-for", parent, NULL));
}
