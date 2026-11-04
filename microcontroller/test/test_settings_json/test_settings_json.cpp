#include <Arduino.h>
#include <ArduinoJson.h>
#include <unity.h>

#include "config/config.h"
#include "settings/settings_json.h"

namespace {
	// Number of scenes the matrix has
	constexpr int SCENE_COUNT = 9;

	// Settings under test, and the JSON the web app sends
	Settings settings;
	JsonDocument doc;

	// Apply a JSON body to the settings, returning the invalid field (nullptr when all are valid)
	const char *applyJson(const char *body) {
		doc.clear();
		deserializeJson(doc, body);
		return applySettingsFromJson(doc.as<JsonObjectConst>(), settings, SCENE_COUNT);
	}
}

// Start each test from the default settings
void setUp() {
	settings = { true, 60, -1, 3000, true, "MERRY CHRISTMAS" };
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
	TEST_ASSERT_EQUAL_STRING("MERRY CHRISTMAS", settings.message);
}

// The message is stored in capitals
void test_message_is_stored_in_capitals() {
	TEST_ASSERT_NULL(applyJson("{\"message\":\"buon natale <\"}"));
	TEST_ASSERT_EQUAL_STRING("BUON NATALE <", settings.message);
}

// The edges of each range are accepted
void test_range_edges_are_accepted() {
	TEST_ASSERT_NULL(applyJson("{\"brightness\":255,\"scene\":8,\"maxMilliamps\":1000}"));
	TEST_ASSERT_EQUAL_UINT8(255, settings.brightness);
	TEST_ASSERT_EQUAL_INT(8, settings.scene);
	TEST_ASSERT_EQUAL_UINT32(config::MIN_MAX_MILLIAMPS, settings.maxMilliamps);
}

// A null field is the same as a missing one
void test_null_field_is_ignored() {
	TEST_ASSERT_NULL(applyJson("{\"scene\":null}"));
	TEST_ASSERT_EQUAL_INT(-1, settings.scene);
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
	TEST_ASSERT_EQUAL_STRING("scene", applyJson("{\"scene\":9}"));
	TEST_ASSERT_EQUAL_STRING("scene", applyJson("{\"scene\":-2}"));
	TEST_ASSERT_EQUAL_STRING("on", applyJson("{\"on\":1}"));
	TEST_ASSERT_EQUAL_STRING("message", applyJson("{\"message\":42}"));
}

// A message longer than the matrix keeps is refused
void test_long_message_is_refused() {
	char body[config::MAX_MESSAGE_LENGTH + 32];
	char message[config::MAX_MESSAGE_LENGTH + 2];
	memset(message, 'A', sizeof(message) - 1);
	message[sizeof(message) - 1] = '\0';
	snprintf(body, sizeof(body), "{\"message\":\"%s\"}", message);
	TEST_ASSERT_EQUAL_STRING("message", applyJson(body));
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
	TEST_ASSERT_EQUAL_STRING("MERRY CHRISTMAS", output["message"].as<const char *>());
}

// Run all the settings JSON tests
int main() {
	UNITY_BEGIN();
	RUN_TEST(test_only_sent_fields_change);
	RUN_TEST(test_message_is_stored_in_capitals);
	RUN_TEST(test_range_edges_are_accepted);
	RUN_TEST(test_null_field_is_ignored);
	RUN_TEST(test_power_limit_above_the_maximum_is_refused);
	RUN_TEST(test_invalid_fields_are_named);
	RUN_TEST(test_long_message_is_refused);
	RUN_TEST(test_invalid_body_changes_nothing);
	RUN_TEST(test_settings_are_written_to_json);
	return UNITY_END();
}