#include <Arduino.h>
#include <unity.h>

#include "config/config.h"
#include "settings/settings_rules.h"

namespace {
	// Number of scenes the matrix has
	constexpr int SCENE_COUNT = 9;

	// Output of the message rules (the longest message and its end)
	char message[256];
}

// Nothing to prepare before each test
void setUp() {}

// Nothing to clean up after each test
void tearDown() {}

/******************************** Holidays ********************************/

// A holiday that doesn't exist goes back to the calendar
void test_unknown_holiday_follows_the_calendar() {
	TEST_ASSERT_EQUAL_INT(1, sanitizeHoliday(1, 2));
	TEST_ASSERT_EQUAL_INT(-1, sanitizeHoliday(2, 2));
	TEST_ASSERT_EQUAL_INT(-1, sanitizeHoliday(-5, 2));
}

// The scene stays within its holiday, and a scene without a fixed holiday rotates
void test_scene_stays_within_its_holiday() {
	const int sceneCounts[] = { 8, 5 };
	int holiday = 0, scene = 0;

	// A valid pair is kept
	sanitizeHolidayScene(1, 4, sceneCounts, 2, holiday, scene);
	TEST_ASSERT_EQUAL_INT(1, holiday);
	TEST_ASSERT_EQUAL_INT(4, scene);

	// A scene past the end of its holiday rotates
	sanitizeHolidayScene(1, 5, sceneCounts, 2, holiday, scene);
	TEST_ASSERT_EQUAL_INT(-1, scene);

	// A scene while the calendar picks the holiday rotates
	sanitizeHolidayScene(-1, 2, sceneCounts, 2, holiday, scene);
	TEST_ASSERT_EQUAL_INT(-1, holiday);
	TEST_ASSERT_EQUAL_INT(-1, scene);
}

// A board saved on Fireworks (scene 8 before the holidays) rotates the Christmas scenes, since Fireworks moved to New Year's Eve
void test_saved_fireworks_rotates_christmas() {
	const int sceneCounts[] = { 8, 5 };
	int holiday = 0, scene = 0;
	sanitizeHolidayScene(0, 8, sceneCounts, 2, holiday, scene);
	TEST_ASSERT_EQUAL_INT(0, holiday);
	TEST_ASSERT_EQUAL_INT(-1, scene);
}

// With the Christmas countdown, the ninth Christmas scene, a board saved on Fireworks shows the countdown instead
void test_saved_fireworks_shows_the_christmas_countdown() {
	const int sceneCounts[] = { 9, 5, 1 };
	int holiday = 0, scene = 0;
	sanitizeHolidayScene(0, 8, sceneCounts, 3, holiday, scene);
	TEST_ASSERT_EQUAL_INT(0, holiday);
	TEST_ASSERT_EQUAL_INT(8, scene);
}

// A color of the custom message that doesn't exist goes back to the default one (white)
void test_unknown_message_color_is_white() {
	TEST_ASSERT_EQUAL_UINT8(6, sanitizeMessageColor(6));
	TEST_ASSERT_EQUAL_UINT8(config::DEFAULT_MESSAGE_COLOR, sanitizeMessageColor(config::MESSAGE_COLOR_COUNT));
	TEST_ASSERT_EQUAL_UINT8(config::DEFAULT_MESSAGE_COLOR, sanitizeMessageColor(-3));
}

// The languages are found by their code, and an unknown one goes back to the default (Italian)
void test_languages_by_code() {
	TEST_ASSERT_EQUAL_INT(ENGLISH, findLanguage("en"));
	TEST_ASSERT_EQUAL_INT(ITALIAN, findLanguage("it"));
	TEST_ASSERT_EQUAL_INT(-1, findLanguage("de"));
	TEST_ASSERT_EQUAL_STRING("en", getLanguageCode(ENGLISH));
	TEST_ASSERT_EQUAL_UINT8(DEFAULT_LANGUAGE, sanitizeLanguage(9));
}

// The default greetings are recognized in every language, and a message written by hand is not
void test_default_greetings_are_recognized() {
	TEST_ASSERT_EQUAL_STRING("BUON ANNO!", getDefaultMessage(NEW_YEAR, ITALIAN));
	TEST_ASSERT_EQUAL_STRING("HELLO!", getDefaultMessage(CUSTOM, ENGLISH));
	TEST_ASSERT_TRUE(isDefaultMessage(CHRISTMAS, "MERRY CHRISTMAS!"));
	TEST_ASSERT_TRUE(isDefaultMessage(CHRISTMAS, "BUON NATALE * MERRY CHRISTMAS"));
	TEST_ASSERT_FALSE(isDefaultMessage(CHRISTMAS, "BUON NATALE A TUTTI"));
	TEST_ASSERT_FALSE(isDefaultMessage(NEW_YEAR, "MERRY CHRISTMAS!"));
}

/******************************** Numbers ********************************/

// A scene that doesn't exist goes back to the rotation
void test_unknown_scene_rotates_all_scenes() {
	TEST_ASSERT_EQUAL_INT(3, sanitizeScene(3, SCENE_COUNT));
	TEST_ASSERT_EQUAL_INT(-1, sanitizeScene(-1, SCENE_COUNT));
	TEST_ASSERT_EQUAL_INT(-1, sanitizeScene(-7, SCENE_COUNT));
	TEST_ASSERT_EQUAL_INT(-1, sanitizeScene(SCENE_COUNT, SCENE_COUNT));
}

// The power limit stays within its allowed range
void test_power_limit_is_kept_in_range() {
	TEST_ASSERT_EQUAL_UINT32(config::MIN_MAX_MILLIAMPS, sanitizeMaxMilliamps(0));
	TEST_ASSERT_EQUAL_UINT32(3000, sanitizeMaxMilliamps(3000));
	TEST_ASSERT_EQUAL_UINT32(config::MAX_MAX_MILLIAMPS, sanitizeMaxMilliamps(70000));
}

/******************************** Message ********************************/

// The message is shown in capitals (the font has no lower case)
void test_message_is_in_capitals() {
	sanitizeMessage("buon natale!", message, sizeof(message));
	TEST_ASSERT_EQUAL_STRING("BUON NATALE!", message);
}

// A long message is cut to the longest one the matrix keeps
void test_long_message_is_cut() {
	char longMessage[300];
	memset(longMessage, 'a', sizeof(longMessage) - 1);
	longMessage[sizeof(longMessage) - 1] = '\0';

	// Sanitize the long message
	sanitizeMessage(longMessage, message, sizeof(message));

	// Expect the longest message, in capitals
	TEST_ASSERT_EQUAL_size_t(config::MAX_MESSAGE_LENGTH, strlen(message));
	TEST_ASSERT_EQUAL_CHAR('A', message[0]);
}

// A small output buffer is never overrun
void test_small_output_is_not_overrun() {
	char small[6];
	sanitizeMessage("merry christmas", small, sizeof(small));
	TEST_ASSERT_EQUAL_STRING("MERRY", small);
}

// Run all the settings tests
int main() {
	UNITY_BEGIN();
	RUN_TEST(test_unknown_holiday_follows_the_calendar);
	RUN_TEST(test_scene_stays_within_its_holiday);
	RUN_TEST(test_saved_fireworks_rotates_christmas);
	RUN_TEST(test_saved_fireworks_shows_the_christmas_countdown);
	RUN_TEST(test_unknown_message_color_is_white);
	RUN_TEST(test_languages_by_code);
	RUN_TEST(test_default_greetings_are_recognized);
	RUN_TEST(test_unknown_scene_rotates_all_scenes);
	RUN_TEST(test_power_limit_is_kept_in_range);
	RUN_TEST(test_message_is_in_capitals);
	RUN_TEST(test_long_message_is_cut);
	RUN_TEST(test_small_output_is_not_overrun);
	return UNITY_END();
}