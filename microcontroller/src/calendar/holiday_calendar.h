#ifndef HOLIDAY_CALENDAR_H
#define HOLIDAY_CALENDAR_H

#include <time.h>

// Which holiday the calendar picks, and the New Year midnight, kept free of the hardware so the native tests can check them

// The holidays, in the order of the web app
enum HolidayIndex {
	CHRISTMAS = 0,
	NEW_YEAR = 1
};
constexpr int HOLIDAY_COUNT = 2;
constexpr int AUTOMATIC_HOLIDAY = -1; // The chosen holiday when the calendar picks it

int getHolidayForDate(const struct tm &now);
int getActiveHoliday(int chosenHoliday, const struct tm &now);
long getSecondsToNewYear(const struct tm &now);
bool isNewYearMoment(const struct tm &now);

#endif