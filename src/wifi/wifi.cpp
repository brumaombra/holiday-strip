#include "wifi.h"

#include <ESPmDNS.h>
#include <WiFi.h>

#include "config/config.h"

namespace {
	// Whether the web server was announced on mDNS since the last connection
	bool isAnnounced = false;
}

// Connect to the Wi-Fi (it reconnects by itself), and set the clock from the network
void setupWiFi() {
	WiFi.mode(WIFI_STA);
	WiFi.setHostname(config::HOSTNAME);
	WiFi.setAutoReconnect(true);
	WiFi.begin(config::WIFI_SSID, config::WIFI_PASSWORD);
	configTzTime(config::TIMEZONE, config::NTP_SERVER, config::NTP_BACKUP_SERVER);
}

// Announce the web server once connected, and again after a reconnection
void manageWiFiConnection() {
	const bool isConnected = WiFi.status() == WL_CONNECTED;
	if (isConnected && !isAnnounced) {
		isAnnounced = true;
		MDNS.addService("http", "tcp", 80);
		Serial.printf("Connected, IP: %s\n", WiFi.localIP().toString().c_str());
	}
	if (!isConnected) {
		isAnnounced = false;
	}
}