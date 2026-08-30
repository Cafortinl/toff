#ifndef DATE_UTILITIES_HPP
#define DATE_UTILITIES_HPP

#include <time.h>

typedef struct {
    time_t start_date;
    time_t end_date;
} date_range;

#define IS_DATE_RANGE_VALID(x) \
    (difftime((x).end_date, (x).start_date) > 0)

#define IS_DATE_RANGE_VALID_INCLUSIVE(x) \
    (difftime((x).end_date, (x).start_date) >= 0)

#define IS_DATE_WITHIN_RANGE(date, range)     \
    (difftime((date),(range).start_date) >= 0 \
    &&                                        \
    difftime((range).end_date, (date)) >= 0)

/*
 *  Returns the last date in the given month.
 *
 *  @param `month` the (zero based) index of the month to check.
 *  @param `year` the year to check.
 *  @returns an `integer` representing the last date of the given month.
 */
int get_month_last_date(int month, int year);

/*
 * Returns a `date_range` struct of a given month containing the offsets needed
 * to completely populate the calendar grid.
 *
 * @param `month` the (one based) index of the range's month.
 * @param `year` the range's year.
 * @returns a `date_range` struct.
 */
date_range generate_calendar_date_range(int month, int year);
#endif
