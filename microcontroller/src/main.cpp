#include <Arduino.h>
#include <gizmo/device.h>
#include <gizmo/log.h>

#include "display/display.h"
#include "player/player.h"
#include "scenes/scenes.h"
#include "server/server.h"
#include "settings/settings.h"
#include "wifi/wifi.h"

namespace {
	// The main loop's copy of the settings, and the revision it was taken at
	Settings settings;
	uint32_t appliedRevision = 0;

	// Take the current settings, and give each holiday greeting its message
	void applySettings() {
		appliedRevision = getSettingsRevision();
		settings = getSettings();
		for (int i = 0; i < HOLIDAY_COUNT; i++) {
			setHolidayMessage(i, settings.messages[i]);
		}
		setMessageColor(settings.messageColor);
		setMatrixLanguage(settings.language);
	}
}

// Setup
void setup() {
	gizmo::initLog(); // Start logging (off in the released firmware, on in the debug build: pio run -e debug)
	setupDisplay(); // Start the LED output

	// Load the saved settings
	int sceneCounts[HOLIDAY_COUNT];
	getSceneCounts(sceneCounts);
	loadSettings(sceneCounts);
	applySettings();

	// Network and web app
	setupWiFi(); // Join the saved network, or open the setup hotspot
	setupServer(); // Serve the web app and its API

	// Factory reset: hold the BOOT button for 10 s to forget the Wi-Fi and the settings
	gizmo::FactoryResetOptions resetOptions;
	resetOptions.clearSettings = [] { clearSettings(); };
	gizmo::setupFactoryReset(resetOptions);

	// Start the first scene
	setupPlayer(settings);
}

// Loop
void loop() {
	manageWiFiConnection();
	gizmo::manageDevice(); // Restart when the web app asks

	// Apply the changes made from the web app
	if (getSettingsRevision() != appliedRevision) {
		applySettings();
	}

	// Draw the next frame when it's due
	updatePlayer(settings);
}