#include "countdown.h"

#include <math.h>
#include <stdio.h>

namespace {
	constexpr int CLOCK_SET_YEAR = 120; // tm_year of 2020: an earlier date means the clock was not set from the network yet
	constexpr long COUNTDOWN_DAYS = 60; // Days before Christmas when the countdown starts
	constexpr int DECEMBER = 11;
	constexpr int NOVEMBER = 10;
	constexpr int CHRISTMAS_DAY = 25;

	// Get the same date at midnight
	struct tm getMidnight(const struct tm &date) {
		struct tm midnight = date;
		midnight.tm_hour = 0;
		midnight.tm_min = 0;
		midnight.tm_sec = 0;
		return midnight;
	}
}

// Check whether the clock was set (from the network); before that, the date is in 1970
bool isClockSet(const struct tm &now) {
	return now.tm_year >= CLOCK_SET_YEAR;
}

// Write the countdown to Christmas shown after the greeting, and return whether there is one (none after Christmas or more than 60 days before)
bool formatChristmasCountdown(const struct tm &now, char *text, size_t length) {
	text[0] = '\0';
	if (!isClockSet(now)) {
		return false;
	}

	// Christmas day itself
	if (now.tm_mon == DECEMBER && now.tm_mday == CHRISTMAS_DAY) {
		snprintf(text, length, " * OGGI E' NATALE!");
		return true;
	}

	// Days until Christmas, rounded (the hour lost when daylight saving time ends would otherwise make it one less)
	struct tm christmas = getMidnight(now);
	christmas.tm_mon = DECEMBER;
	christmas.tm_mday = CHRISTMAS_DAY;
	struct tm today = getMidnight(now);
	const long days = lround(difftime(mktime(&christmas), mktime(&today)) / 86400.0);

	// No countdown after Christmas, or too early
	if (days <= 0 || days > COUNTDOWN_DAYS) {
		return false;
	}

	// One day, or several
	if (days == 1) {
		snprintf(text, length, " * MANCA 1 GIORNO A NATALE!");
	} else {
		snprintf(text, length, " * MANCANO %ld GIORNI A NATALE!", days);
	}
	return true;
}

// Get the year the fireworks show: the current one, or the coming one from November on (0 before the clock is set)
int getUpcomingYear(const struct tm &now) {
	if (!isClockSet(now)) {
		return 0;
	}
	return now.tm_year + 1900 + (now.tm_mon >= NOVEMBER ? 1 : 0);
}