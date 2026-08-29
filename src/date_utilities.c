#include "date_utilities.h"

int get_month_last_date(int month, int year) {
    struct tm date_builder = {
        .tm_year = year - 1900,
        .tm_mon = month + 1,
        .tm_mday = -1
    };
    mktime(&date_builder);

    return date_builder.tm_mday + 1;
}

date_range generate_calendar_date_range(int month, int year) {
    // Find weekday of the first day of the month.
    struct tm start_date_builder = {
        .tm_year = year - 1900,
        .tm_mon = month - 1,
        .tm_mday = 1
    };
    time_t start_date = mktime(&start_date_builder);

    int start_date_week_day = localtime(&start_date)->tm_wday;
    /*
     *  Offsetting the first day to query depending of the first day of the
     *  month.
     *
     *  tm_wday: 1  2  3  4  5  6  0
     *  offset: [0][1][2][3][4][5][6]
     */
    if (start_date_week_day != 1) {
        struct tm start_date_builder = {
            .tm_year = year - 1900,
            .tm_mon = month - 1,
            .tm_mday = 1 - (start_date_week_day == 0 ? 6 : start_date_week_day - 1)
        };
        start_date = mktime(&start_date_builder);
    }

    int last_date = get_month_last_date(2, 2026);
    struct tm end_date_builder = {
        .tm_year = 2026 - 1900,
        .tm_mon = 2,
        .tm_mday = last_date
    };
    time_t end_date = mktime(&end_date_builder);

    int end_date_week_day = localtime(&end_date)->tm_wday;
    /*
     *  Offsetting the last day to query depending of the last day of the
     *  month.
     *
     *  tm_wday: 1  2  3  4  5  6  0
     *  offset: [6][5][4][3][2][1][0]
     */
    if (end_date_week_day != 0) {
        struct tm end_date_builder = {
            .tm_year = year - 1900,
            .tm_mon = month - 1,
            .tm_mday = last_date + (end_date_week_day == 0 ? 0 : 7 - end_date_week_day)
        };
        end_date = mktime(&end_date_builder);
    }

    return (date_range) { .start_date = start_date, .end_date = end_date };
}
