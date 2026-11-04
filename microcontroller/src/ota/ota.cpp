#include "ota.h"

#include <ArduinoOTA.h>
#include <gizmo/log.h>

#include "config/config.h"

namespace {
	// Whether wireless updates are on
	bool isOtaEnabled = false;
}

// Accept password-protected firmware updates over Wi-Fi (off when no password is set)
void setupOta(void (*onStart)()) {
	if (config::OTA_PASSWORD[0] == '\0') {
		gizmo::printLogln("Wireless updates are off: set PIO_OTA_PASSWORD in platformio_secrets.ini");
		return;
	}

	// Start listening for updates (mDNS is already run by the Wi-Fi setup, so upload by IP address)
	ArduinoOTA.setHostname(config::HOSTNAME);
	ArduinoOTA.setPassword(config::OTA_PASSWORD);
	ArduinoOTA.setMdnsEnabled(false);
	ArduinoOTA.onStart(onStart);
	ArduinoOTA.begin();
	isOtaEnabled = true;
}

// Handle a pending update (call on every loop iteration)
void handleOta() {
	if (isOtaEnabled) {
		ArduinoOTA.handle();
	}
}