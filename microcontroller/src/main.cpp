#include <Arduino.h>
#include <gizmo/device.h>
#include <gizmo/log.h>

#include "display/display.h"
#include "ota/ota.h"
#include "player/player.h"
#include "scenes/scenes.h"
#include "server/server.h"
#include "settings/settings.h"
#include "wifi/wifi.h"

namespace {
	// The main loop's copy of the settings, and the revision it was taken at
	Settings settings;
	uint32_t appliedRevision = 0;

	// Take the current settings, and give the greeting its message
	void applySettings() {
		appliedRevision = getSettingsRevision();
		settings = getSettings();
		setGreetingMessage(settings.message);
		setCountdownEnabled(settings.countdown);
	}

	// Turn the LEDs off while an OTA update runs
	void handleOtaStart() {
		showBlackFrame(settings.maxMilliamps);
	}
}

// Setup
void setup() {
	gizmo::initLog(true); // Start logging
	setupDisplay(); // Start the LED output

	// Load the saved settings
	loadSettings(getSceneCount());
	applySettings();

	// Network, web app and wireless updates
	setupWiFi(); // Join the saved network, or open the setup hotspot
	setupServer(); // Serve the web app and its API
	setupOta(handleOtaStart); // Accept wireless updates

	// Start the first scene
	setupPlayer(settings);
}

// Loop
void loop() {
	manageWiFiConnection();
	gizmo::manageDevice(); // Restart when the web app asks
	handleOta();

	// Apply the changes made from the web app
	if (getSettingsRevision() != appliedRevision) {
		applySettings();
	}

	// Draw the next frame when it's due
	updatePlayer(settings);
}