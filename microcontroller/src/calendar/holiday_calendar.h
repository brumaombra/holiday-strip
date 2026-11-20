#ifndef HOLIDAY_CALENDAR_H
#define HOLIDAY_CALENDAR_H

#include <time.h>

// Which holiday the calendar picks, and the New Year midnight, kept free of the hardware so the native tests can check them

// The holidays, in the order they were added (the saved settings keep their index: the web app lists the custom message last). The custom message is picked like a holiday, but the calendar never picks it
enum HolidayIndex {
	CHRISTMAS = 0,
	NEW_YEAR = 1,
	CUSTOM = 2,
	EASTER = 3
};
constexpr int HOLIDAY_COUNT = 4;
constexpr int AUTOMATIC_HOLIDAY = -1; // The chosen holiday when the calendar picks it

void getEasterSunday(int year, int &month, int &day);
int getHolidayForDate(const struct tm &now);
int getActiveHoliday(int chosenHoliday, const struct tm &now);
long getSecondsToNewYear(const struct tm &now);
bool isNewYearMoment(const struct tm &now);

#endif