#ifndef TOFF_DAY_DETAIL_VIEW_H
#define TOFF_DAY_DETAIL_VIEW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TOFF_TYPE_DAY_DETAIL_VIEW (toff_day_detail_view_get_type())
G_DECLARE_FINAL_TYPE(ToffDayDetailView, toff_day_detail_view, TOFF, DAY_DETAIL_VIEW, GtkWindow)

GtkWidget* toff_day_detail_view_new(GtkWindow *parent);

G_END_DECLS
#endif
