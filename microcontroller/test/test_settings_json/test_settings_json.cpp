#include <Arduino.h>
#include <ArduinoJson.h>
#include <unity.h>

#include "config/config.h"
#include "settings/settings_json.h"

namespace {
	// Number of scenes of each holiday (Christmas, New Year's Eve, the custom message), and the holiday playing now
	constexpr int SCENE_COUNTS[HOLIDAY_COUNT] = { 8, 5, 1 };
	int activeHoliday = CHRISTMAS;

	// Settings under test, and the JSON the web app sends
	Settings settings;
	JsonDocument doc;

	// Apply a JSON body to the settings, returning the invalid field (nullptr when all are valid)
	const char *applyJson(const char *body) {
		doc.clear();
		deserializeJson(doc, body);
		return applySettingsFromJson(doc.as<JsonObjectConst>(), settings, SCENE_COUNTS, activeHoliday);
	}
}

// Start each test from the default settings
void setUp() {
	settings = { true, 60, AUTOMATIC_HOLIDAY, -1, 3000, true, { "MERRY CHRISTMAS", "HAPPY NEW YEAR", "CIAO" }, 0 };
	activeHoliday = CHRISTMAS;
}

// Nothing to clean up after each test
void tearDown() {}

/******************************** Valid changes ********************************/

// Only the fields that are sent change
void test_only_sent_fields_change() {
	TEST_ASSERT_NULL(applyJson("{\"brightness\":80,\"on\":false}"));
	TEST_ASSERT_EQUAL_UINT8(80, settings.brightness);
	TEST_ASSERT_FALSE(settings.on);
	TEST_ASSERT_EQUAL_INT(-1, settings.scene);
	TEST_ASSERT_EQUAL_STRING("MERRY CHRISTMAS", settings.messages[CHRISTMAS]);
}

// The messages are stored in capitals, and a null one is left as it is
void test_messages_are_stored_in_capitals() {
	TEST_ASSERT_NULL(applyJson("{\"messages\":[null,\"buon anno <\"]}"));
	TEST_ASSERT_EQUAL_STRING("MERRY CHRISTMAS", settings.messages[CHRISTMAS]);
	TEST_ASSERT_EQUAL_STRING("BUON ANNO <", settings.messages[NEW_YEAR]);
}

// The edges of each range are accepted
void test_range_edges_are_accepted() {
	TEST_ASSERT_NULL(applyJson("{\"brightness\":255,\"holiday\":1,\"scene\":4,\"maxMilliamps\":1000}"));
	TEST_ASSERT_EQUAL_UINT8(255, settings.brightness);
	TEST_ASSERT_EQUAL_INT(NEW_YEAR, settings.holiday);
	TEST_ASSERT_EQUAL_INT(4, settings.scene);
	TEST_ASSERT_EQUAL_UINT32(config::MIN_MAX_MILLIAMPS, settings.maxMilliamps);
}

// A null field is the same as a missing one
void test_null_field_is_ignored() {
	TEST_ASSERT_NULL(applyJson("{\"scene\":null}"));
	TEST_ASSERT_EQUAL_INT(-1, settings.scene);
}

/******************************** Holidays and scenes ********************************/

// Another holiday without a scene rotates through its scenes
void test_new_holiday_rotates_its_scenes() {
	settings.holiday = CHRISTMAS;
	settings.scene = 3;
	TEST_ASSERT_NULL(applyJson("{\"holiday\":1}"));
	TEST_ASSERT_EQUAL_INT(NEW_YEAR, settings.holiday);
	TEST_ASSERT_EQUAL_INT(-1, settings.scene);
}

// Sending the same holiday again keeps the chosen scene
void test_same_holiday_keeps_the_scene() {
	settings.holiday = CHRISTMAS;
	settings.scene = 3;
	TEST_ASSERT_NULL(applyJson("{\"holiday\":0}"));
	TEST_ASSERT_EQUAL_INT(3, settings.scene);
}

// A scene chosen while the calendar picks the holiday pins the holiday playing now
void test_chosen_scene_pins_the_active_holiday() {
	activeHoliday = NEW_YEAR;
	TEST_ASSERT_NULL(applyJson("{\"scene\":2}"));
	TEST_ASSERT_EQUAL_INT(NEW_YEAR, settings.holiday);
	TEST_ASSERT_EQUAL_INT(2, settings.scene);
}

// The scene is checked against the holiday it plays in (New Year's Eve has fewer scenes)
void test_scene_is_checked_against_its_holiday() {
	activeHoliday = NEW_YEAR;
	TEST_ASSERT_EQUAL_STRING("scene", applyJson("{\"scene\":7}"));
	TEST_ASSERT_NULL(applyJson("{\"holiday\":0,\"scene\":7}"));
	TEST_ASSERT_EQUAL_INT(CHRISTMAS, settings.holiday);
	TEST_ASSERT_EQUAL_INT(7, settings.scene);
}

// Back to the calendar with the rotation
void test_automatic_holiday_rotates() {
	settings.holiday = NEW_YEAR;
	settings.scene = 1;
	TEST_ASSERT_NULL(applyJson("{\"holiday\":-1}"));
	TEST_ASSERT_EQUAL_INT(AUTOMATIC_HOLIDAY, settings.holiday);
	TEST_ASSERT_EQUAL_INT(-1, settings.scene);
}

// The custom message is picked like a holiday, and has its own message and color
void test_custom_message_has_its_message_and_color() {
	TEST_ASSERT_NULL(applyJson("{\"holiday\":2,\"messages\":[null,null,\"saldi!\"],\"messageColor\":6}"));
	TEST_ASSERT_EQUAL_INT(CUSTOM, settings.holiday);
	TEST_ASSERT_EQUAL_INT(-1, settings.scene);
	TEST_ASSERT_EQUAL_STRING("SALDI!", settings.messages[CUSTOM]);
	TEST_ASSERT_EQUAL_STRING("MERRY CHRISTMAS", settings.messages[CHRISTMAS]);
	TEST_ASSERT_EQUAL_UINT8(6, settings.messageColor);
}

/******************************** Invalid changes ********************************/

// A power limit above the supply's safe limit is refused (this used to be up to 60 A)
void test_power_limit_above_the_maximum_is_refused() {
	char body[64];
	snprintf(body, sizeof(body), "{\"maxMilliamps\":%lu}", (unsigned long)config::MAX_MAX_MILLIAMPS + 1);
	TEST_ASSERT_EQUAL_STRING("maxMilliamps", applyJson(body));
	TEST_ASSERT_EQUAL_UINT32(3000, settings.maxMilliamps);
}

// Values out of range or of the wrong type are refused, by field name
void test_invalid_fields_are_named() {
	TEST_ASSERT_EQUAL_STRING("brightness", applyJson("{\"brightness\":256}"));
	TEST_ASSERT_EQUAL_STRING("brightness", applyJson("{\"brightness\":12.5}"));
	TEST_ASSERT_EQUAL_STRING("scene", applyJson("{\"scene\":8}"));
	TEST_ASSERT_EQUAL_STRING("holiday", applyJson("{\"holiday\":3}"));
	TEST_ASSERT_EQUAL_STRING("messageColor", applyJson("{\"messageColor\":7}"));
	TEST_ASSERT_EQUAL_STRING("messageColor", applyJson("{\"messageColor\":-1}"));
	TEST_ASSERT_EQUAL_STRING("holiday", applyJson("{\"holiday\":-2}"));
	TEST_ASSERT_EQUAL_STRING("scene", applyJson("{\"scene\":-2}"));
	TEST_ASSERT_EQUAL_STRING("on", applyJson("{\"on\":1}"));
	TEST_ASSERT_EQUAL_STRING("messages", applyJson("{\"messages\":\"HELLO\"}"));
	TEST_ASSERT_EQUAL_STRING("messages", applyJson("{\"messages\":[42]}"));
	TEST_ASSERT_EQUAL_STRING("messages", applyJson("{\"messages\":[\"A\",\"B\",\"C\",\"D\"]}"));
}

// A message longer than the matrix keeps is refused
void test_long_message_is_refused() {
	char body[config::MAX_MESSAGE_LENGTH + 32];
	char message[config::MAX_MESSAGE_LENGTH + 2];
	memset(message, 'A', sizeof(message) - 1);
	message[sizeof(message) - 1] = '\0';
	snprintf(body, sizeof(body), "{\"messages\":[\"%s\"]}", message);
	TEST_ASSERT_EQUAL_STRING("messages", applyJson(body));
}

// One invalid field leaves every setting as it was
void test_invalid_body_changes_nothing() {
	TEST_ASSERT_EQUAL_STRING("scene", applyJson("{\"brightness\":10,\"scene\":42}"));
	TEST_ASSERT_EQUAL_UINT8(60, settings.brightness);
}

/******************************** Output ********************************/

// The settings are written with the names the web app reads
void test_settings_are_written_to_json() {
	JsonDocument output;
	writeSettingsToJson(output.to<JsonObject>(), settings);
	TEST_ASSERT_TRUE(output["on"].as<bool>());
	TEST_ASSERT_EQUAL_INT(60, output["brightness"].as<int>());
	TEST_ASSERT_EQUAL_INT(-1, output["scene"].as<int>());
	TEST_ASSERT_EQUAL_INT(3000, output["maxMilliamps"].as<int>());
	TEST_ASSERT_TRUE(output["countdown"].as<bool>());
	TEST_ASSERT_EQUAL_INT(-1, output["holiday"].as<int>());
	TEST_ASSERT_EQUAL_STRING("MERRY CHRISTMAS", output["messages"][0].as<const char *>());
	TEST_ASSERT_EQUAL_STRING("HAPPY NEW YEAR", output["messages"][1].as<const char *>());
	TEST_ASSERT_EQUAL_STRING("CIAO", output["messages"][2].as<const char *>());
	TEST_ASSERT_EQUAL_INT(0, output["messageColor"].as<int>());
}

// Run all the settings JSON tests
int main() {
	UNITY_BEGIN();
	RUN_TEST(test_only_sent_fields_change);
	RUN_TEST(test_messages_are_stored_in_capitals);
	RUN_TEST(test_range_edges_are_accepted);
	RUN_TEST(test_null_field_is_ignored);
	RUN_TEST(test_new_holiday_rotates_its_scenes);
	RUN_TEST(test_same_holiday_keeps_the_scene);
	RUN_TEST(test_chosen_scene_pins_the_active_holiday);
	RUN_TEST(test_scene_is_checked_against_its_holiday);
	RUN_TEST(test_automatic_holiday_rotates);
	RUN_TEST(test_custom_message_has_its_message_and_color);
	RUN_TEST(test_power_limit_above_the_maximum_is_refused);
	RUN_TEST(test_invalid_fields_are_named);
	RUN_TEST(test_long_message_is_refused);
	RUN_TEST(test_invalid_body_changes_nothing);
	RUN_TEST(test_settings_are_written_to_json);
	return UNITY_END();
}