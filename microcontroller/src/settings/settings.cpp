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
	const char SCENE_KEY[] = "scene";
	const char MAX_MILLIAMPS_KEY[] = "maxma";
	const char COUNTDOWN_KEY[] = "countdown";
	const char MESSAGE_KEY[] = "message";

	// Current settings and their revision (shared by the main loop and the web server task)
	Settings currentSettings;
	uint32_t revision = 0;
	SemaphoreHandle_t settingsMutex = nullptr;
}

// Load the saved settings (call once in setup, before the web server starts)
void loadSettings(int sceneCount) {
	settingsMutex = xSemaphoreCreateMutex();
	Preferences preferences;
	char savedMessage[config::MAX_MESSAGE_LENGTH + 1];

	// Read the saved settings over the defaults (missing keys keep their default)
	preferences.begin(NAMESPACE, true);
	currentSettings.on = preferences.getBool(ON_KEY, config::DEFAULT_ON);
	currentSettings.brightness = preferences.getUChar(BRIGHTNESS_KEY, config::DEFAULT_BRIGHTNESS);
	currentSettings.scene = sanitizeScene(preferences.getInt(SCENE_KEY, config::DEFAULT_SCENE), sceneCount);
	currentSettings.maxMilliamps = sanitizeMaxMilliamps(preferences.getUInt(MAX_MILLIAMPS_KEY, config::DEFAULT_MAX_MILLIAMPS));
	currentSettings.countdown = preferences.getBool(COUNTDOWN_KEY, config::DEFAULT_COUNTDOWN);
	if (preferences.getString(MESSAGE_KEY, savedMessage, sizeof(savedMessage)) == 0) {
		gizmo::copyString(savedMessage, config::DEFAULT_MESSAGE, sizeof(savedMessage));
	}
	preferences.end();

	// Keep the message within the rules (a scene that no longer exists and a too high power limit were fixed above)
	sanitizeMessage(savedMessage, currentSettings.message, sizeof(currentSettings.message));
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
	const bool isSaved = preferences.putBool(ON_KEY, settings.on) > 0 && preferences.putUChar(BRIGHTNESS_KEY, settings.brightness) > 0 && preferences.putInt(SCENE_KEY, settings.scene) > 0 && preferences.putUInt(MAX_MILLIAMPS_KEY, settings.maxMilliamps) > 0 && preferences.putBool(COUNTDOWN_KEY, settings.countdown) > 0 && (preferences.putString(MESSAGE_KEY, settings.message) > 0 || settings.message[0] == '\0'); // An empty message writes 0 bytes
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
