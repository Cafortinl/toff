#ifndef TOFF_CALENDAR_VIEW_H
#define TOFF_CALENDAR_VIEW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*
 *  Type Declaration
 *
 *  For more info: https://docs.gtk.org/gobject/tutorial.html
 */
#define TOFF_TYPE_CALENDAR_VIEW (toff_calendar_view_get_type())
G_DECLARE_FINAL_TYPE(ToffCalendarView, toff_calendar_view, TOFF, CALENDAR_VIEW, GtkBox)

GtkWidget* toff_calendar_view_new(void);

G_END_DECLS
#endif
