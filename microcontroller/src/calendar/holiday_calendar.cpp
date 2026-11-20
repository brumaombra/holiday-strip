#include "holiday_calendar.h"

#include <math.h>

#include "calendar/countdown.h"

namespace {
	constexpr int JANUARY = 0;
	constexpr int DECEMBER = 11;
	constexpr int NEW_YEAR_FIRST_DAY = 27; // New Year's Eve plays from December 27, once Christmas is over
	constexpr long MOMENT_SECONDS = 60; // The New Year moment: the last minute of the year and the first of the new one
	constexpr long EASTER_FIRST_DAY = -7; // Easter plays from Palm Sunday, a week before Easter Sunday
	constexpr long EASTER_LAST_DAY = 1; // to Easter Monday

	// Get the days from one date to another (local midnights, rounded: a day can be 23 or 25 hours long)
	long getDaysBetween(int year, int month, int day, const struct tm &now) {
		struct tm from = {};
		from.tm_year = year - 1900;
		from.tm_mon = month;
		from.tm_mday = day;
		from.tm_hour = 12;
		from.tm_isdst = -1;
		struct tm to = now;
		to.tm_hour = 12;
		to.tm_min = 0;
		to.tm_sec = 0;
		to.tm_isdst = -1;
		return lround(difftime(mktime(&to), mktime(&from)) / 86400.0);
	}

	// Check whether a date is in the Easter days: from Palm Sunday to Easter Monday
	bool isEasterTime(const struct tm &now) {
		int month = 0, day = 0;
		getEasterSunday(now.tm_year + 1900, month, day);
		const long days = getDaysBetween(now.tm_year + 1900, month, day, now);
		return days >= EASTER_FIRST_DAY && days <= EASTER_LAST_DAY;
	}
}

// Get the date of Easter Sunday in a year, in the Gregorian calendar (month 0 to 11): the anonymous Gregorian algorithm (Meeus/Jones/Butcher)
void getEasterSunday(int year, int &month, int &day) {
	const int a = year % 19, b = year / 100, c = year % 100;
	const int d = b / 4, e = b % 4, f = (b + 8) / 25, g = (b - f + 1) / 3;
	const int h = (19 * a + b - d - g + 15) % 30;
	const int i = c / 4, k = c % 4;
	const int l = (32 + 2 * e + 2 * i - h - k) % 7;
	const int m = (a + 11 * h + 22 * l) / 451;
	month = (h + l - 7 * m + 114) / 31 - 1;
	day = (h + l - 7 * m + 114) % 31 + 1;
}

// Get the holiday of a date: New Year's Eve from December 27 to January 1, Easter from Palm Sunday to Easter Monday, Christmas the rest of the year (and before the clock is set)
int getHolidayForDate(const struct tm &now) {
	if (!isClockSet(now)) {
		return CHRISTMAS;
	}
	const bool isEndOfDecember = now.tm_mon == DECEMBER && now.tm_mday >= NEW_YEAR_FIRST_DAY;
	const bool isNewYearsDay = now.tm_mon == JANUARY && now.tm_mday == 1;
	if (isEndOfDecember || isNewYearsDay) {
		return NEW_YEAR;
	}
	return isEasterTime(now) ? EASTER : CHRISTMAS;
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