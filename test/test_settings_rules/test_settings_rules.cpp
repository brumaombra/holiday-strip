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

/******************************** Numbers ********************************/

// The brightness stays within 0 to 255
void test_brightness_is_kept_in_range() {
	TEST_ASSERT_EQUAL_UINT8(0, sanitizeBrightness(-5));
	TEST_ASSERT_EQUAL_UINT8(80, sanitizeBrightness(80));
	TEST_ASSERT_EQUAL_UINT8(255, sanitizeBrightness(300));
}

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
	RUN_TEST(test_brightness_is_kept_in_range);
	RUN_TEST(test_unknown_scene_rotates_all_scenes);
	RUN_TEST(test_power_limit_is_kept_in_range);
	RUN_TEST(test_message_is_in_capitals);
	RUN_TEST(test_long_message_is_cut);
	RUN_TEST(test_small_output_is_not_overrun);
	return UNITY_END();
}