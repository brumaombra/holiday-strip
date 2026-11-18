#include <Arduino.h>
#include <stdlib.h>
#include <time.h>
#include <unity.h>

#include "calendar/holiday_calendar.h"
#include "config/config.h"

namespace {
	// Build a local date and time (month 1 to 12), with the fields mktime fills in
	struct tm makeTime(int year, int month, int day, int hour = 12, int minute = 0, int second = 0) {
		struct tm date = {};
		date.tm_year = year - 1900;
		date.tm_mon = month - 1;
		date.tm_mday = day;
		date.tm_hour = hour;
		date.tm_min = minute;
		date.tm_sec = second;
		date.tm_isdst = -1;
		mktime(&date);
		return date;
	}
}

// Use the timezone of the matrix (Central European Time, with daylight saving time)
void setUp() {
	setenv("TZ", config::TIMEZONE, 1);
	tzset();
}

// Nothing to clean up after each test
void tearDown() {}

/******************************** The holiday of a date ********************************/

// Christmas plays until December 26
void test_christmas_until_boxing_day() {
	TEST_ASSERT_EQUAL_INT(CHRISTMAS, getHolidayForDate(makeTime(2026, 11, 18)));
	TEST_ASSERT_EQUAL_INT(CHRISTMAS, getHolidayForDate(makeTime(2026, 12, 25)));
	TEST_ASSERT_EQUAL_INT(CHRISTMAS, getHolidayForDate(makeTime(2026, 12, 26, 23, 59)));
}

// New Year's Eve plays from December 27 to January 1
void test_new_year_from_december_27_to_january_1() {
	TEST_ASSERT_EQUAL_INT(NEW_YEAR, getHolidayForDate(makeTime(2026, 12, 27, 0, 0)));
	TEST_ASSERT_EQUAL_INT(NEW_YEAR, getHolidayForDate(makeTime(2026, 12, 31, 23, 59)));
	TEST_ASSERT_EQUAL_INT(NEW_YEAR, getHolidayForDate(makeTime(2027, 1, 1, 18)));
	TEST_ASSERT_EQUAL_INT(CHRISTMAS, getHolidayForDate(makeTime(2027, 1, 2)));
}

// Before the clock is set, Christmas plays
void test_christmas_before_the_clock_is_set() {
	TEST_ASSERT_EQUAL_INT(CHRISTMAS, getHolidayForDate(makeTime(1970, 1, 1)));
}

// The calendar never picks the custom message, on any day of the year
void test_calendar_never_picks_the_custom_message() {
	for (int month = 1; month <= 12; month++) {
		for (int day = 1; day <= 28; day += 3) {
			TEST_ASSERT_NOT_EQUAL(CUSTOM, getHolidayForDate(makeTime(2026, month, day)));
		}
	}
	TEST_ASSERT_EQUAL_INT(CUSTOM, getActiveHoliday(CUSTOM, makeTime(2026, 12, 31)));
}

// A chosen holiday plays whatever the date
void test_chosen_holiday_wins_over_the_date() {
	TEST_ASSERT_EQUAL_INT(NEW_YEAR, getActiveHoliday(NEW_YEAR, makeTime(2026, 7, 1)));
	TEST_ASSERT_EQUAL_INT(CHRISTMAS, getActiveHoliday(CHRISTMAS, makeTime(2026, 12, 31)));
	TEST_ASSERT_EQUAL_INT(NEW_YEAR, getActiveHoliday(AUTOMATIC_HOLIDAY, makeTime(2026, 12, 31)));
}

/******************************** Midnight ********************************/

// The seconds left until midnight of January 1
void test_seconds_to_new_year() {
	TEST_ASSERT_EQUAL_INT32(10, getSecondsToNewYear(makeTime(2026, 12, 31, 23, 59, 50)));
	TEST_ASSERT_EQUAL_INT32(86400, getSecondsToNewYear(makeTime(2026, 12, 31, 0, 0, 0)));
	TEST_ASSERT_EQUAL_INT32(-1, getSecondsToNewYear(makeTime(1970, 1, 1)));
}

// The midnight count still works across the end of summer time (October has a 25 hour day)
void test_seconds_to_new_year_from_summer_time() {
	const long fromSummer = getSecondsToNewYear(makeTime(2026, 10, 1, 0, 0, 0));
	TEST_ASSERT_EQUAL_INT32(92L * 86400 + 3600, fromSummer); // 92 days, plus the hour given back
}

// The New Year moment: the last minute of the year and the first one of the next
void test_new_year_moment() {
	TEST_ASSERT_FALSE(isNewYearMoment(makeTime(2026, 12, 31, 23, 58, 59)));
	TEST_ASSERT_TRUE(isNewYearMoment(makeTime(2026, 12, 31, 23, 59, 0)));
	TEST_ASSERT_TRUE(isNewYearMoment(makeTime(2027, 1, 1, 0, 0, 30)));
	TEST_ASSERT_FALSE(isNewYearMoment(makeTime(2027, 1, 1, 0, 1, 0)));
	TEST_ASSERT_FALSE(isNewYearMoment(makeTime(1970, 1, 1, 0, 0, 0)));
}

// Run all the holiday calendar tests
int main() {
	UNITY_BEGIN();
	RUN_TEST(test_christmas_until_boxing_day);
	RUN_TEST(test_new_year_from_december_27_to_january_1);
	RUN_TEST(test_christmas_before_the_clock_is_set);
	RUN_TEST(test_calendar_never_picks_the_custom_message);
	RUN_TEST(test_chosen_holiday_wins_over_the_date);
	RUN_TEST(test_seconds_to_new_year);
	RUN_TEST(test_seconds_to_new_year_from_summer_time);
	RUN_TEST(test_new_year_moment);
	return UNITY_END();
}