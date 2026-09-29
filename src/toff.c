#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

#include "toff_calendar_day_cell.h"

#define GRID_ROWS    6
#define GRID_COLUMNS 7

/**
 *  Stores all the information to be shown in the calendar grid.
 */
// typedef struct {
//     date_range dates;
//     day_event_information day_information[GRID_ROWS * GRID_COLUMNS];
// } calendar_day_information;
//
// void calendar_day_information_free(calendar_day_information *calendar_information) {
//     for (size_t i = 0; i < GRID_ROWS * GRID_COLUMNS; ++i) {
//         day_event_information_free(&(calendar_information->day_information[i]));
//     }
//     free(calendar_information);
// }
// EndSection: Data Containers
//
// calendar_day_information* populate_calendar_information(sqlite_database_administrator *dba, int month, int year) {
//     if (!dba) {
//         fprintf(stderr, "populate_calendar_information - Error: no valid sqlite_database_administrator struct was provided.\n");
//         return NULL;
//     }
//
//     if (!dba->database) {
//         fprintf(stderr, "populate_calendar_information - Error: no valid database connection.\n");
//         return NULL;
//     }
//
//     date_range dates = generate_calendar_date_range(month, year);
//     char start_date_str[11];
//     strftime(start_date_str, 11, "%F", localtime(&(dates.start_date)));
//     char end_date_str[11];
//     strftime(end_date_str, 11, "%F", localtime(&(dates.end_date)));
//
//     printf("Date range: %s - %s\n", start_date_str, end_date_str);
//
//     calendar_day_information *calendar_information = malloc(sizeof(calendar_day_information) * 1);
//     calendar_information->dates = dates;
//
//     result_information *holidays = get_holidays_in_date_range(dba, dates);
//     result_information *events = get_events_in_date_range(dba, dates);
//     result_information *vacations = get_vacations_in_date_range(dba, dates);
//
//     struct tm date_iterator_builder = *(localtime(&dates.start_date));
//     time_t date_iterator = mktime(&date_iterator_builder);
//     int day_index = 0;
//     while (date_iterator <= dates.end_date) {
//         day_event_information current_day;
//
//         current_day.date = date_iterator;
//
//         current_day.is_weekend
//             = localtime(&date_iterator)->tm_wday == 0 || localtime(&date_iterator)->tm_wday == 6;
//
//         for (size_t i = 0; i < events->size; ++i) {
//             event current_event = ((event*) events->data)[i];
//
//             if (IS_DATE_WITHIN_RANGE(date_iterator, current_event.dates)) {
//                 current_day.events.is_event = true;
//                 goto can_have_vacations_check;
//             }
//         }
//
//         for (size_t i = 0; i < holidays->size; ++i) {
//             holiday current_holiday = ((holiday*) holidays->data)[i];
//
//             if (IS_DATE_WITHIN_RANGE(date_iterator, current_holiday.dates)) {
//                 current_day.events.is_holiday = true;
//                 goto can_have_vacations_check;
//             }
//         }
//
// can_have_vacations_check:
//         if (current_day.is_weekend || current_day.events.is_event || current_day.events.is_holiday)
//             goto end;
//
//         current_day.events.vacations.data = malloc(vacations->size * sizeof(vacation));
//         current_day.events.vacations.size = vacations->size;
//         for (size_t i = 0; i < vacations->size; ++i) {
//             vacation current_vacation = ((vacation*) vacations->data)[i];
//
//             if (!IS_DATE_WITHIN_RANGE(date_iterator, current_vacation.dates))
//                 continue;
//
//             memcpy(&(current_day.events.vacations.data[current_day.events.vacations.count]), &current_vacation, sizeof(vacation) * 1);
//             current_day.events.vacations.data[current_day.events.vacations.count].employee_name = strdup(current_vacation.employee_name);
//             ++current_day.events.vacations.count;
//         }
//
//         if (!current_day.events.vacations.count) {
//            free(current_day.events.vacations.data);
//            current_day.events.vacations.data = NULL;
//            current_day.events.vacations.size = 0;
//            current_day.events.vacations.count = 0;
//         }
//
// end:
//         calendar_information->day_information[day_index] = current_day;
//
//         date_iterator_builder.tm_mday += 1;
//         date_iterator = mktime(&date_iterator_builder);
//         ++day_index;
//     }
//
//     result_information_free(holidays, &holiday_result_extra_processing);
//     result_information_free(events, &event_result_extra_processing);
//     result_information_free(vacations, &vacation_result_extra_processing);
//
//     return calendar_information;
// }

static void app_activate(GApplication *app) {
    GtkBuilder *builder;
    GtkWidget *app_window;

    builder = gtk_builder_new_from_resource("/org/loveless/toff/main_screen.ui");

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
