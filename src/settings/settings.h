#pragma once

#include <Arduino.h>

// Settings of the matrix, saved in flash
struct Settings {
	bool on;
	uint8_t brightness;
	int scene; // -1 rotates through all the scenes
	uint32_t maxMilliamps;
	bool countdown;
	String message;
};

void loadSettings(Settings &settings, int sceneCount);
void saveSettings(const Settings &settings);