#include "wifi.h"

#include <WiFi.h>
#include <gizmo/log.h>
#include <gizmo/wifi_setup.h>

#include "config/config.h"
#include "server/server.h"

namespace {
	// Whether the address was logged since the last connection
	bool isAddressLogged = false;
}

// Join the saved network, or open the setup hotspot, and set the clock from the network (call before setupServer)
void setupWiFi() {
	gizmo::setupWifi(server, { config::ACCESS_POINT_NAME, config::HOSTNAME, config::SETUP_PAGE_PATH, config::ASSET_PATH_PREFIX });

	// Set the clock from the network (after the Wi-Fi started, so the network stack is up; it syncs once connected)
	configTzTime(config::TIMEZONE, config::NTP_SERVER, config::NTP_BACKUP_SERVER);
}

// Keep the Wi-Fi connected, and log the address once connected (call on every loop iteration)
void manageWiFiConnection() {
	gizmo::manageWifi();

	// Log the address once per connection
	const bool isConnected = gizmo::isWifiConnected();
	if (isConnected && !isAddressLogged) {
		gizmo::printLogfln("Connected, open http://%s.local or http://%s", config::HOSTNAME, WiFi.localIP().toString().c_str());
	}
	isAddressLogged = isConnected;
}