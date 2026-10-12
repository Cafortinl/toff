#include "toff_day_detail_view.h"

#include "date_utilities.h"
#include "glib-object.h"
#include "gtk/gtk.h"
#include "calendar_event.h"
#include "vacation.h"

struct _ToffDayDetailView {
    GtkWidget parent;

    GtkWidget *day_label;

    GtkWidget *event_information;
    GtkWidget *event_type_tag;
    GtkWidget *event_name;
    GtkWidget *start_date_label;
    GtkWidget *end_date_label;

    GtkWidget *vacation_information;
    GtkWidget *vacation_list;

    enum day_event_types event_type;
    union {
        event *calendar_event;
        GArray *vacations;
    } events;
    GDateTime *date;
};

G_DEFINE_TYPE(ToffDayDetailView, toff_day_detail_view, GTK_TYPE_WINDOW)

typedef enum {
    PROP_EVENT_TYPE = 1,
    PROP_CALENDAR_EVENT,
    PROP_VACATIONS,
    PROP_DATE,
    N_PROPERTIES
} ToffDayDetailViewProperty;

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void update_event_information(ToffDayDetailView *self) {
    if (!self->events.calendar_event)
        return;

    switch ((enum day_event_types) self->event_type) {
        case DE_EVENT:
            gtk_label_set_text(GTK_LABEL(self->event_type_tag), "Event");
            gtk_widget_add_css_class(GTK_WIDGET(self->event_type_tag), "pill-blue");
            gtk_widget_remove_css_class(GTK_WIDGET(self->event_type_tag), "pill-green");
            break;

        case DE_HOLIDAY:
            gtk_label_set_text(GTK_LABEL(self->event_type_tag), "Holiday");
            gtk_widget_remove_css_class(GTK_WIDGET(self->event_type_tag), "pill-blue");
            gtk_widget_add_css_class(GTK_WIDGET(self->event_type_tag), "pill-green");
            break;

        default:
            break;
    }

    gtk_label_set_text(
        GTK_LABEL(self->start_date_label),
        g_date_time_format(
            g_date_time_new_from_unix_utc(self->events.calendar_event->dates.start_date),
            "%F"
        )
    );

    gtk_label_set_text(
        GTK_LABEL(self->end_date_label),
        g_date_time_format(
            g_date_time_new_from_unix_utc(self->events.calendar_event->dates.end_date),
            "%F"
        )
    );

    gtk_label_set_text(GTK_LABEL(self->event_name), self->events.calendar_event->name);
    gtk_widget_set_visible(GTK_WIDGET(self->event_information), TRUE);
}

static GtkWidget* create_vacation_list_item(vacation *vacation_information) {
    date_range vacation_dates = vacation_information->dates;
    GtkWidget *content_box;
    GtkWidget *name_label;
    GtkWidget *info_label;

    content_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);

    name_label = gtk_label_new(g_strdup(vacation_information->employee_name));
    gtk_widget_set_hexpand(GTK_WIDGET(name_label), TRUE);
    gtk_widget_set_halign(GTK_WIDGET(name_label), GTK_ALIGN_START);

    info_label = gtk_label_new(
        g_strdup_printf(
            "On vacation until: %s",
            g_date_time_format(
                g_date_time_new_from_unix_utc(vacation_dates.end_date),
                "%F"
            )
        )
    );
    gtk_widget_set_hexpand(GTK_WIDGET(info_label), TRUE);
    gtk_widget_set_halign(GTK_WIDGET(info_label), GTK_ALIGN_END);
    gtk_widget_add_css_class(GTK_WIDGET(info_label), "pill");
    gtk_widget_add_css_class(GTK_WIDGET(info_label), "pill-blue");

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
                &g_array_index(self->events.vacations, vacation, i)
            )
        );
    }

    gtk_widget_set_visible(GTK_WIDGET(self->vacation_information), TRUE);
}

static void free_events(ToffDayDetailView *self) {
    switch (self->event_type) {
        case DE_HOLIDAY:
        case DE_EVENT:
            if (self->events.calendar_event) {
                g_clear_pointer(&self->events.calendar_event->name, g_free);
                g_clear_pointer(&self->events.calendar_event, g_free);
            }
            break;

        case DE_VACATION:
            g_clear_pointer(&self->events.vacations, g_array_unref);
            break;

        default:
            break;
    }
}

static void cleanup_events(ToffDayDetailView *self, bool clear_event_type) {
    free_events(self);

    if (self->event_information)
        gtk_widget_set_visible(GTK_WIDGET(self->event_information), FALSE);

    if (self->vacation_information)
        gtk_widget_set_visible(GTK_WIDGET(self->vacation_information), FALSE);

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

        case PROP_CALENDAR_EVENT:
            cleanup_events(self, false);
            event *to_set = g_value_get_pointer(value);
            self->events.calendar_event = g_new0(event, 1);
            *(self->events.calendar_event) = *to_set;
            self->events.calendar_event->name = g_strdup(to_set->name);
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

        case PROP_CALENDAR_EVENT:
            g_value_set_pointer(value, self->events.calendar_event);
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

    free_events(self);
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

    obj_properties[PROP_CALENDAR_EVENT] =
        g_param_spec_pointer(
            "event",
            "Event",
            "Name of the event in the day cell's day (when it applies).",
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
    gtk_widget_class_bind_template_child(widget_class, ToffDayDetailView, event_information);
    gtk_widget_class_bind_template_child(widget_class, ToffDayDetailView, event_type_tag);
    gtk_widget_class_bind_template_child(widget_class, ToffDayDetailView, event_name);
    gtk_widget_class_bind_template_child(widget_class, ToffDayDetailView, start_date_label);
    gtk_widget_class_bind_template_child(widget_class, ToffDayDetailView, end_date_label);
    gtk_widget_class_bind_template_child(widget_class, ToffDayDetailView, vacation_information);
    gtk_widget_class_bind_template_child(widget_class, ToffDayDetailView, vacation_list);
}

static void toff_day_detail_view_init(ToffDayDetailView *self) {
    gtk_widget_init_template(GTK_WIDGET(self));
}

GtkWidget* toff_day_detail_view_new(GtkWindow *parent) {
    return GTK_WIDGET(g_object_new(TOFF_TYPE_DAY_DETAIL_VIEW, "transient-for", parent, NULL));
}
