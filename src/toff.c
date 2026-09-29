#include <gtk/gtk.h>
#include <stdio.h>

#include "toff_calendar_day_cell.h"
#include "toff_calendar_view.h"

static void app_activate(GApplication *app) {
    GtkBuilder *builder;
    GtkWidget *app_window;

    g_type_ensure(TOFF_TYPE_CALENDAR_DAY_CELL);
    g_type_ensure(TOFF_TYPE_CALENDAR_VIEW);

    builder = gtk_builder_new_from_resource("/org/loveless/toff/toff_main_screen.ui");

    app_window = GTK_WIDGET(gtk_builder_get_object(builder, "app_window"));
    gtk_window_set_application(GTK_WINDOW(app_window), GTK_APPLICATION(app));

    gtk_window_present(GTK_WINDOW(app_window));
    g_object_unref(builder);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int app_status;

    app = gtk_application_new("org.loveless.toff", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(app_activate), NULL);
    app_status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return app_status;
}
