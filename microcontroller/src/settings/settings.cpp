#include "settings.h"

#include <Arduino.h>
#include <Preferences.h>
#include <gizmo/text.h>

#include "settings/settings_rules.h"

namespace {
	// NVS namespace (the project name; namespaces fit 15 characters) and keys
	const char NAMESPACE[] = "holiday-strip";
	const char ON_KEY[] = "on";
	const char BRIGHTNESS_KEY[] = "brightness";
	const char HOLIDAY_KEY[] = "holiday";
	const char SCENE_KEY[] = "scene";
	const char MAX_MILLIAMPS_KEY[] = "maxma";
	const char COUNTDOWN_KEY[] = "countdown";
	const char *const MESSAGE_KEYS[HOLIDAY_COUNT] = { "message", "msg-newyear", "msg-custom" }; // One message per holiday ("message" is the Christmas one, from before the holidays)
	const char *const DEFAULT_MESSAGES[HOLIDAY_COUNT] = { config::DEFAULT_MESSAGE, config::DEFAULT_NEW_YEAR_MESSAGE, config::DEFAULT_CUSTOM_MESSAGE };
	const char MESSAGE_COLOR_KEY[] = "msg-color";

	// Current settings and their revision (shared by the main loop and the web server task)
	Settings currentSettings;
	uint32_t revision = 0;
	SemaphoreHandle_t settingsMutex = nullptr;
}

// Load the saved settings (call once in setup, before the web server starts; sceneCounts has one entry per holiday)
void loadSettings(const int *sceneCounts) {
	settingsMutex = xSemaphoreCreateMutex();
	Preferences preferences;
	char savedMessage[config::MAX_MESSAGE_LENGTH + 1];

	// Read the saved settings over the defaults (missing keys keep their default)
	preferences.begin(NAMESPACE, true);
	currentSettings.on = preferences.getBool(ON_KEY, config::DEFAULT_ON);
	currentSettings.brightness = preferences.getUChar(BRIGHTNESS_KEY, config::DEFAULT_BRIGHTNESS);
	currentSettings.maxMilliamps = sanitizeMaxMilliamps(preferences.getUInt(MAX_MILLIAMPS_KEY, config::DEFAULT_MAX_MILLIAMPS));
	currentSettings.countdown = preferences.getBool(COUNTDOWN_KEY, config::DEFAULT_COUNTDOWN);
	currentSettings.messageColor = sanitizeMessageColor(preferences.getUChar(MESSAGE_COLOR_KEY, config::DEFAULT_MESSAGE_COLOR));

	// Read the holiday and the scene: a scene saved before the holidays existed was a Christmas one, so it stays on Christmas
	const long scene = preferences.getInt(SCENE_KEY, config::DEFAULT_SCENE);
	const long holiday = preferences.isKey(HOLIDAY_KEY) ? preferences.getInt(HOLIDAY_KEY, config::DEFAULT_HOLIDAY) : (scene >= 0 ? CHRISTMAS : config::DEFAULT_HOLIDAY);
	sanitizeHolidayScene(holiday, scene, sceneCounts, HOLIDAY_COUNT, currentSettings.holiday, currentSettings.scene);

	// Read the greeting of each holiday, kept within the rules
	for (int i = 0; i < HOLIDAY_COUNT; i++) {
		if (preferences.getString(MESSAGE_KEYS[i], savedMessage, sizeof(savedMessage)) == 0) {
			gizmo::copyString(savedMessage, DEFAULT_MESSAGES[i], sizeof(savedMessage));
		}
		sanitizeMessage(savedMessage, currentSettings.messages[i], sizeof(currentSettings.messages[i]));
	}
	preferences.end();
}

// Get a copy of the current settings (safe from any task)
Settings getSettings() {
	xSemaphoreTake(settingsMutex, portMAX_DELAY);
	const Settings settings = currentSettings;
	xSemaphoreGive(settingsMutex);
	return settings;
}

// Get the revision of the settings, which changes on every update (to notice a change without copying them)
uint32_t getSettingsRevision() {
	xSemaphoreTake(settingsMutex, portMAX_DELAY);
	const uint32_t currentRevision = revision;
	xSemaphoreGive(settingsMutex);
	return currentRevision;
}

// Save new settings and make them current (safe from any task; false, and nothing changes, if they couldn't be saved)
bool updateSettings(const Settings &settings) {
	Preferences preferences;

	// Save the settings (NVS skips the values that didn't change)
	if (!preferences.begin(NAMESPACE, false)) {
		return false;
	}
	bool isSaved = preferences.putBool(ON_KEY, settings.on) > 0 && preferences.putUChar(BRIGHTNESS_KEY, settings.brightness) > 0 && preferences.putInt(HOLIDAY_KEY, settings.holiday) > 0 && preferences.putInt(SCENE_KEY, settings.scene) > 0 && preferences.putUInt(MAX_MILLIAMPS_KEY, settings.maxMilliamps) > 0 && preferences.putBool(COUNTDOWN_KEY, settings.countdown) > 0 && preferences.putUChar(MESSAGE_COLOR_KEY, settings.messageColor) > 0;
	for (int i = 0; i < HOLIDAY_COUNT && isSaved; i++) {
		isSaved = preferences.putString(MESSAGE_KEYS[i], settings.messages[i]) > 0 || settings.messages[i][0] == '\0'; // An empty message writes 0 bytes
	}
	preferences.end();
	if (!isSaved) {
		return false;
	}

	// Make them current
	xSemaphoreTake(settingsMutex, portMAX_DELAY);
	currentSettings = settings;
	revision += 1;
	xSemaphoreGive(settingsMutex);
	return true;
}

// Forget the saved settings
bool clearSettings() {
	Preferences preferences;
	if (!preferences.begin(NAMESPACE, false)) {
		return false;
	}
	const bool isCleared = preferences.clear();
	preferences.end();
	return isCleared;
}
