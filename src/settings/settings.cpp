#include "settings.h"

#include <Preferences.h>

#include "config/config.h"

namespace {
	// NVS namespace and keys (kept from the first version, so the saved settings survive updates)
	const char NAMESPACE[] = "matrix";
	const char ON_KEY[] = "on";
	const char BRIGHTNESS_KEY[] = "brightness";
	const char SCENE_KEY[] = "scene";
	const char MAX_MILLIAMPS_KEY[] = "maxma";
	const char COUNTDOWN_KEY[] = "countdown";
	const char MESSAGE_KEY[] = "message";

	Preferences preferences;
}

// Load the saved settings over the defaults (missing keys keep their default)
void loadSettings(Settings &settings, int sceneCount) {
	preferences.begin(NAMESPACE, true);
	settings.on = preferences.getBool(ON_KEY, config::DEFAULT_ON);
	settings.brightness = preferences.getUChar(BRIGHTNESS_KEY, config::DEFAULT_BRIGHTNESS);
	settings.scene = preferences.getInt(SCENE_KEY, config::DEFAULT_SCENE);
	settings.maxMilliamps = preferences.getUInt(MAX_MILLIAMPS_KEY, config::DEFAULT_MAX_MILLIAMPS);
	settings.countdown = preferences.getBool(COUNTDOWN_KEY, config::DEFAULT_COUNTDOWN);
	settings.message = preferences.getString(MESSAGE_KEY, config::DEFAULT_MESSAGE);
	preferences.end();

	// A scene that no longer exists goes back to the rotation
	if (settings.scene >= sceneCount) {
		settings.scene = -1;
	}
}

// Save the settings
void saveSettings(const Settings &settings) {
	preferences.begin(NAMESPACE, false);
	preferences.putBool(ON_KEY, settings.on);
	preferences.putUChar(BRIGHTNESS_KEY, settings.brightness);
	preferences.putInt(SCENE_KEY, settings.scene);
	preferences.putUInt(MAX_MILLIAMPS_KEY, settings.maxMilliamps);
	preferences.putBool(COUNTDOWN_KEY, settings.countdown);
	preferences.putString(MESSAGE_KEY, settings.message);
	preferences.end();
}