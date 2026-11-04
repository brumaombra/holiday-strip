#include "ota.h"

#include <ArduinoOTA.h>

#include "config/config.h"

// Accept firmware updates over Wi-Fi (it also starts mDNS with the hostname)
void setupOta(void (*onStart)()) {
	ArduinoOTA.setHostname(config::HOSTNAME);
	ArduinoOTA.onStart(onStart);
	ArduinoOTA.begin();
}

// Handle a pending update
void handleOta() {
	ArduinoOTA.handle();
}