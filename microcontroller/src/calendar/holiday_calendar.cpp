#include "holiday_calendar.h"

#include <math.h>

#include "calendar/countdown.h"

namespace {
	constexpr int JANUARY = 0;
	constexpr int DECEMBER = 11;
	constexpr int NEW_YEAR_FIRST_DAY = 27; // New Year's Eve plays from December 27, once Christmas is over
	constexpr long MOMENT_SECONDS = 60; // The New Year moment: the last minute of the year and the first of the new one
}

// Get the holiday of a date: New Year's Eve from December 27 to January 1, Christmas the rest of the year (and before the clock is set)
int getHolidayForDate(const struct tm &now) {
	if (!isClockSet(now)) {
		return CHRISTMAS;
	}
	const bool isEndOfDecember = now.tm_mon == DECEMBER && now.tm_mday >= NEW_YEAR_FIRST_DAY;
	const bool isNewYearsDay = now.tm_mon == JANUARY && now.tm_mday == 1;
	return isEndOfDecember || isNewYearsDay ? NEW_YEAR : CHRISTMAS;
}

// Get the holiday that plays: the chosen one, or the one of the date when the choice is automatic
int getActiveHoliday(int chosenHoliday, const struct tm &now) {
	if (chosenHoliday >= 0 && chosenHoliday < HOLIDAY_COUNT) {
		return chosenHoliday;
	}
	return getHolidayForDate(now);
}

// Get the seconds left until the next midnight of January 1 (-1 before the clock is set)
long getSecondsToNewYear(const struct tm &now) {
	if (!isClockSet(now)) {
		return -1;
	}

	// Midnight of the coming January 1
	struct tm newYear = {};
	newYear.tm_year = now.tm_year + 1;
	newYear.tm_mon = JANUARY;
	newYear.tm_mday = 1;
	newYear.tm_isdst = -1; // Let mktime find daylight saving time
	struct tm today = now;
	return lround(difftime(mktime(&newYear), mktime(&today)));
}

// Check whether it's the New Year moment: the last minute of December 31 or the first minute of January 1
bool isNewYearMoment(const struct tm &now) {
	if (!isClockSet(now)) {
		return false;
	}

	// The first minute of the year
	if (now.tm_mon == JANUARY && now.tm_mday == 1 && now.tm_hour == 0 && now.tm_min == 0) {
		return true;
	}

	// The last minute of the year
	const long secondsLeft = getSecondsToNewYear(now);
	return secondsLeft > 0 && secondsLeft <= MOMENT_SECONDS;
}