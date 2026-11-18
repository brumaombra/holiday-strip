#ifndef SETTINGS_H
#define SETTINGS_H

#include <stddef.h>
#include <stdint.h>

#include "calendar/holiday_calendar.h"
#include "config/config.h"
#include "config/language.h"

// Settings of the matrix, saved in flash
struct Settings {
	bool on;
	uint8_t brightness;
	int holiday; // -1 follows the calendar
	int scene; // -1 rotates through all the scenes of the holiday; a chosen scene always comes with a chosen holiday
	uint32_t maxMilliamps;
	bool countdown; // The Christmas countdown after the Christmas greeting
	char messages[HOLIDAY_COUNT][config::MAX_MESSAGE_LENGTH + 1]; // The greeting of each holiday, and the custom message, in capitals
	uint8_t messageColor; // Color of the custom message (0 to MESSAGE_COLOR_COUNT - 1)
	uint8_t language; // Language of the texts on the matrix (Language), the one picked on the dashboard
};

// Load the saved settings (call once in setup, before the web server starts; sceneCounts has one entry per holiday)
void loadSettings(const int *sceneCounts);

// Get a copy of the current settings (safe from any task)
Settings getSettings();

// Get the revision of the settings, which changes on every update (to notice a change without copying them)
uint32_t getSettingsRevision();

// Save new settings and make them current (safe from any task; false, and nothing changes, if they couldn't be saved)
bool updateSettings(const Settings &settings);

// Forget the saved settings (a factory reset: the defaults come back at the next boot)
bool clearSettings();

#endif