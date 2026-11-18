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
	TEST_ASSERT_EQUAL_INT32(-1, getDaysToChristmas(makeDate(1970, 1, 1)));
}

// The days left until Christmas, on any day of the year
void test_countdown_counts_the_days() {
	TEST_ASSERT_EQUAL_INT32(24, getDaysToChristmas(makeDate(2026, 12, 1)));
	TEST_ASSERT_EQUAL_INT32(1, getDaysToChristmas(makeDate(2026, 12, 24)));
	TEST_ASSERT_EQUAL_INT32(176, getDaysToChristmas(makeDate(2026, 7, 2)));
}

// On Christmas day there are no days left
void test_christmas_day_is_zero() {
	TEST_ASSERT_EQUAL_INT32(0, getDaysToChristmas(makeDate(2026, 12, 25)));
}

// After Christmas it counts to the next one
void test_after_christmas_counts_to_the_next() {
	TEST_ASSERT_EQUAL_INT32(364, getDaysToChristmas(makeDate(2026, 12, 26)));
	TEST_ASSERT_EQUAL_INT32(359, getDaysToChristmas(makeDate(2026, 12, 31)));
}

// On the day the clocks go back (one hour longer), the count is still a whole number of days
void test_countdown_survives_the_end_of_summer_time() {
	TEST_ASSERT_EQUAL_INT32(60, getDaysToChristmas(makeDate(2025, 10, 26)));
}

// The sentence in Italian: several days, the last one, and Christmas day
void test_sentence_in_italian() {
	formatChristmasCountdown(24, ITALIAN, text, sizeof(text));
	TEST_ASSERT_EQUAL_STRING("MANCANO 24 GIORNI A NATALE!", text);
	formatChristmasCountdown(1, ITALIAN, text, sizeof(text));
	TEST_ASSERT_EQUAL_STRING("MANCA 1 GIORNO A NATALE!", text);
	formatChristmasCountdown(0, ITALIAN, text, sizeof(text));
	TEST_ASSERT_EQUAL_STRING("OGGI E' NATALE!", text);
}

// The sentence in English, when the dashboard is in English
void test_sentence_in_english() {
	formatChristmasCountdown(24, ENGLISH, text, sizeof(text));
	TEST_ASSERT_EQUAL_STRING("24 DAYS TO CHRISTMAS!", text);
	formatChristmasCountdown(1, ENGLISH, text, sizeof(text));
	TEST_ASSERT_EQUAL_STRING("1 DAY TO CHRISTMAS!", text);
	formatChristmasCountdown(0, ENGLISH, text, sizeof(text));
	TEST_ASSERT_EQUAL_STRING("IT'S CHRISTMAS DAY!", text);
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
	RUN_TEST(test_christmas_day_is_zero);
	RUN_TEST(test_after_christmas_counts_to_the_next);
	RUN_TEST(test_countdown_survives_the_end_of_summer_time);
	RUN_TEST(test_sentence_in_italian);
	RUN_TEST(test_sentence_in_english);
	RUN_TEST(test_fireworks_show_the_current_year);
	RUN_TEST(test_fireworks_show_the_coming_year_from_november);
	RUN_TEST(test_no_year_before_the_clock_is_set);
	return UNITY_END();
}