#include <Arduino.h>
#include <stdlib.h>
#include <time.h>
#include <unity.h>

#include "calendar/countdown.h"
#include "config/config.h"

namespace {
	// Text written by the countdown
	char text[48];

	// Build a local date at noon (month 1 to 12), with the fields mktime fills in
	struct tm makeDate(int year, int month, int day) {
		struct tm date = {};
		date.tm_year = year - 1900;
		date.tm_mon = month - 1;
		date.tm_mday = day;
		date.tm_hour = 12;
		date.tm_isdst = -1;
		mktime(&date);
		return date;
	}
}

// Use the timezone of the matrix (Central European Time, with daylight saving time)
void setUp() {
	setenv("TZ", config::TIMEZONE, 1);
	tzset();
	text[0] = '\0';
}

// Nothing to clean up after each test
void tearDown() {}

/******************************** Christmas countdown ********************************/

// Before the clock is set from the network the date is in 1970: no countdown
void test_no_countdown_before_the_clock_is_set() {
	TEST_ASSERT_FALSE(formatChristmasCountdown(makeDate(1970, 1, 1), text, sizeof(text)));
	TEST_ASSERT_EQUAL_STRING("", text);
}

// Some days before Christmas, the countdown says how many
void test_countdown_counts_the_days() {
	TEST_ASSERT_TRUE(formatChristmasCountdown(makeDate(2026, 12, 1), text, sizeof(text)));
	TEST_ASSERT_EQUAL_STRING(" * MANCANO 24 GIORNI A NATALE!", text);
}

// The day before Christmas uses the singular
void test_last_day_is_singular() {
	TEST_ASSERT_TRUE(formatChristmasCountdown(makeDate(2026, 12, 24), text, sizeof(text)));
	TEST_ASSERT_EQUAL_STRING(" * MANCA 1 GIORNO A NATALE!", text);
}

// On Christmas day, the greeting says it is today
void test_christmas_day_says_today() {
	TEST_ASSERT_TRUE(formatChristmasCountdown(makeDate(2026, 12, 25), text, sizeof(text)));
	TEST_ASSERT_EQUAL_STRING(" * OGGI E' NATALE!", text);
}

// After Christmas there is no countdown (it would count to a Christmas in the past)
void test_no_countdown_after_christmas() {
	TEST_ASSERT_FALSE(formatChristmasCountdown(makeDate(2026, 12, 26), text, sizeof(text)));
	TEST_ASSERT_FALSE(formatChristmasCountdown(makeDate(2026, 12, 31), text, sizeof(text)));
}

// The countdown starts 60 days before Christmas, not earlier
void test_countdown_starts_60_days_before() {
	TEST_ASSERT_TRUE(formatChristmasCountdown(makeDate(2026, 10, 26), text, sizeof(text)));
	TEST_ASSERT_EQUAL_STRING(" * MANCANO 60 GIORNI A NATALE!", text);
	TEST_ASSERT_FALSE(formatChristmasCountdown(makeDate(2026, 10, 25), text, sizeof(text)));
}

// On the day the clocks go back (one hour longer), the count is still a whole number of days
void test_countdown_survives_the_end_of_summer_time() {
	TEST_ASSERT_TRUE(formatChristmasCountdown(makeDate(2025, 10, 26), text, sizeof(text)));
	TEST_ASSERT_EQUAL_STRING(" * MANCANO 60 GIORNI A NATALE!", text);
}

/******************************** New Year ********************************/

// The fireworks show the current year until October
void test_fireworks_show_the_current_year() {
	TEST_ASSERT_EQUAL_INT(2026, getUpcomingYear(makeDate(2026, 10, 31)));
	TEST_ASSERT_EQUAL_INT(2027, getUpcomingYear(makeDate(2027, 1, 1)));
}

// From November, the fireworks show the coming year
void test_fireworks_show_the_coming_year_from_november() {
	TEST_ASSERT_EQUAL_INT(2027, getUpcomingYear(makeDate(2026, 11, 1)));
	TEST_ASSERT_EQUAL_INT(2027, getUpcomingYear(makeDate(2026, 12, 31)));
}

// Before the clock is set, there is no year to show
void test_no_year_before_the_clock_is_set() {
	TEST_ASSERT_EQUAL_INT(0, getUpcomingYear(makeDate(1970, 1, 1)));
}

// Run all the countdown tests
int main() {
	UNITY_BEGIN();
	RUN_TEST(test_no_countdown_before_the_clock_is_set);
	RUN_TEST(test_countdown_counts_the_days);
	RUN_TEST(test_last_day_is_singular);
	RUN_TEST(test_christmas_day_says_today);
	RUN_TEST(test_no_countdown_after_christmas);
	RUN_TEST(test_countdown_starts_60_days_before);
	RUN_TEST(test_countdown_survives_the_end_of_summer_time);
	RUN_TEST(test_fireworks_show_the_current_year);
	RUN_TEST(test_fireworks_show_the_coming_year_from_november);
	RUN_TEST(test_no_year_before_the_clock_is_set);
	return UNITY_END();
}