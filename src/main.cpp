#include <Arduino.h>

#include "display/display.h"
#include "ota/ota.h"
#include "player/player.h"
#include "scenes/scenes.h"
#include "settings/settings.h"
#include "web/web_server.h"
#include "wifi/wifi.h"

namespace {
	// The settings of the matrix, shared by the web server and the player
	Settings settings;

	// Turn the LEDs off while an OTA update runs
	void handleOtaStart() {
		showBlackFrame(settings.maxMilliamps);
	}
}

// Setup
void setup() {
	Serial.begin(115200);
	setupDisplay();

	// Load the settings and give the greeting its message
	loadSettings(settings, getSceneCount());
	setGreetingMessage(settings.message);
	setCountdownEnabled(settings.countdown);

	// Network, web control and OTA updates
	setupWiFi();
	setupWebServer(settings);
	setupOta(handleOtaStart);

	// Start the first scene
	setupPlayer(settings);
}

// Loop
void loop() {
	manageWiFiConnection();
	handleWebClients();
	handleOta();
	updatePlayer(settings);
}