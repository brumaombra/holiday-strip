#ifndef WIFI_H
#define WIFI_H

// Join the saved network, or open the setup hotspot, and set the clock from the network (call before setupServer)
void setupWiFi();

// Keep the Wi-Fi connected, and log the address once connected (call on every loop iteration)
void manageWiFiConnection();

#endif