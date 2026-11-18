#include "settings_json.h"

#include <string.h>

#include "settings/settings_rules.h"

namespace {
	// Read an optional boolean field (false if it's there with the wrong type)
	bool readBoolField(JsonVariantConst value, bool &output) {
		if (value.isNull()) {
			return true;
		}
		if (!value.is<bool>()) {
			return false;
		}
		output = value.as<bool>();
		return true;
	}

	// Read an optional whole number field within a range (false if it's there with the wrong type or out of range)
	bool readIntegerField(JsonVariantConst value, long minValue, long maxValue, long &output) {
		if (value.isNull()) {
			return true;
		}
		if (!value.is<long>()) {
			return false;
		}

		// Check the range
		const long number = value.as<long>();
		if (number < minValue || number > maxValue) {
			return false;
		}
		output = number;
		return true;
	}
}

// Write the settings into a JSON object
void writeSettingsToJson(JsonObject json, const Settings &settings) {
	json["on"] = settings.on;
	json["brightness"] = settings.brightness;
	json["holiday"] = settings.holiday;
	json["scene"] = settings.scene;
	json["maxMilliamps"] = settings.maxMilliamps;
	json["messageColor"] = settings.messageColor;
	json["language"] = getLanguageCode(settings.language);

	// The greeting of each holiday, in the order of the holidays
	JsonArray messages = json["messages"].to<JsonArray>();
	for (const auto &message : settings.messages) {
		messages.add(message);
	}
}

// Apply the fields of a JSON object to the settings, all or nothing (returns the name of the first invalid field, or nullptr)
const char *applySettingsFromJson(JsonObjectConst json, Settings &settings, const int *sceneCounts, int activeHoliday) {
	Settings updated = settings;

	// Read the switch
	if (!readBoolField(json["on"], updated.on)) {
		return "on";
	}

	// Read the brightness
	long brightness = updated.brightness;
	if (!readIntegerField(json["brightness"], 0, 255, brightness)) {
		return "brightness";
	}
	updated.brightness = (uint8_t)brightness;

	// Read the holiday (-1 follows the calendar)
	long holiday = updated.holiday;
	if (!readIntegerField(json["holiday"], AUTOMATIC_HOLIDAY, HOLIDAY_COUNT - 1, holiday)) {
		return "holiday";
	}
	const bool isHolidayChanged = holiday != updated.holiday;
	updated.holiday = (int)holiday;

	// Read the scene (-1 rotates through all of them) within the holiday it plays in
	JsonVariantConst sceneField = json["scene"];
	if (!sceneField.isNull()) {
		const int sceneHoliday = updated.holiday >= 0 ? updated.holiday : activeHoliday;
		long scene = updated.scene;
		if (!readIntegerField(sceneField, -1, sceneCounts[sceneHoliday] - 1, scene)) {
			return "scene";
		}
		updated.scene = (int)scene;

		// A chosen scene only makes sense in a fixed holiday: pin the one playing now
		if (updated.scene >= 0) {
			updated.holiday = sceneHoliday;
		}
	} else if (isHolidayChanged) {
		// Another holiday without a scene: rotate through its scenes
		updated.scene = -1;
	}

	// Read the color of the custom message
	long messageColor = updated.messageColor;
	if (!readIntegerField(json["messageColor"], 0, config::MESSAGE_COLOR_COUNT - 1, messageColor)) {
		return "messageColor";
	}
	updated.messageColor = (uint8_t)messageColor;

	// Read the power limit
	long maxMilliamps = updated.maxMilliamps;
	if (!readIntegerField(json["maxMilliamps"], config::MIN_MAX_MILLIAMPS, config::MAX_MAX_MILLIAMPS, maxMilliamps)) {
		return "maxMilliamps";
	}
	updated.maxMilliamps = (uint32_t)maxMilliamps;

	// Read the language of the matrix, by its code
	JsonVariantConst languageField = json["language"];
	if (!languageField.isNull()) {
		const int language = languageField.is<const char *>() ? findLanguage(languageField.as<const char *>()) : -1;
		if (language < 0) {
			return "language";
		}

		// Translate the greetings that are still a default one (the ones written by hand stay as they are)
		if (language != updated.language) {
			for (int i = 0; i < HOLIDAY_COUNT; i++) {
				if (isDefaultMessage(i, updated.messages[i])) {
					sanitizeMessage(getDefaultMessage(i, (uint8_t)language), updated.messages[i], sizeof(updated.messages[i]));
				}
			}
		}
		updated.language = (uint8_t)language;
	}

	// Read the greetings, one per holiday (null leaves one as it is), stored in capitals
	JsonVariantConst messages = json["messages"];
	if (!messages.isNull()) {
		if (!messages.is<JsonArrayConst>() || messages.size() > HOLIDAY_COUNT) {
			return "messages";
		}
		for (size_t i = 0; i < messages.size(); i++) {
			// Skip the greetings that don't change
			JsonVariantConst message = messages[i];
			if (message.isNull()) {
				continue;
			}

			// Check and store the greeting
			if (!message.is<const char *>() || strlen(message.as<const char *>()) > config::MAX_MESSAGE_LENGTH) {
				return "messages";
			}
			sanitizeMessage(message.as<const char *>(), updated.messages[i], sizeof(updated.messages[i]));
		}
	}

	// Every field is valid: apply them all
	settings = updated;
	return nullptr;
}