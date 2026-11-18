#include "countdown.h"

#include <math.h>
#include <stdio.h>

namespace {
	constexpr int CLOCK_SET_YEAR = 120; // tm_year of 2020: an earlier date means the clock was not set from the network yet
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

// Get the days left until the next Christmas (0 on Christmas day, the next year's after it; -1 before the clock is set)
long getDaysToChristmas(const struct tm &now) {
	if (!isClockSet(now)) {
		return -1;
	}

	// This year's Christmas, or next year's once it has passed
	struct tm today = getMidnight(now);
	struct tm christmas = getMidnight(now);
	christmas.tm_mon = DECEMBER;
	christmas.tm_mday = CHRISTMAS_DAY;
	if (now.tm_mon == DECEMBER && now.tm_mday > CHRISTMAS_DAY) {
		christmas.tm_year += 1;
	}

	// Rounded, since the hour given back when summer time ends would otherwise make it one less
	return lround(difftime(mktime(&christmas), mktime(&today)) / 86400.0);
}

// Write the countdown sentence in the language of the matrix: the days left, or that it's Christmas today
void formatChristmasCountdown(long days, uint8_t language, char *text, size_t length) {
	const bool isEnglish = language == ENGLISH;
	if (days == 0) {
		snprintf(text, length, isEnglish ? "IT'S CHRISTMAS DAY!" : "OGGI E' NATALE!");
	} else if (days == 1) {
		snprintf(text, length, isEnglish ? "1 DAY TO CHRISTMAS!" : "MANCA 1 GIORNO A NATALE!");
	} else {
		snprintf(text, length, isEnglish ? "%ld DAYS TO CHRISTMAS!" : "MANCANO %ld GIORNI A NATALE!", days);
	}
}

// Get the year the fireworks show: the current one, or the coming one from November on (0 before the clock is set)
int getUpcomingYear(const struct tm &now) {
	if (!isClockSet(now)) {
		return 0;
	}
	return now.tm_year + 1900 + (now.tm_mon >= NOVEMBER ? 1 : 0);
}