#ifndef SETTINGS_H
#define SETTINGS_H

#include <stddef.h>
#include <stdint.h>

#include "config/config.h"

// Settings of the matrix, saved in flash
struct Settings {
	bool on;
	uint8_t brightness;
	int scene; // -1 rotates through all the scenes
	uint32_t maxMilliamps;
	bool countdown;
	char message[config::MAX_MESSAGE_LENGTH + 1]; // In capitals
};

// Load the saved settings (call once in setup, before the web server starts)
void loadSettings(int sceneCount);

// Get a copy of the current settings (safe from any task)
Settings getSettings();

// Get the revision of the settings, which changes on every update (to notice a change without copying them)
uint32_t getSettingsRevision();

// Save new settings and make them current (safe from any task; false, and nothing changes, if they couldn't be saved)
bool updateSettings(const Settings &settings);

// Forget the saved settings (a factory reset: the defaults come back at the next boot)
bool clearSettings();

#endif