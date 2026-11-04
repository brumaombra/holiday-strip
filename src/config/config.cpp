#include "config.h"

#ifndef PIO_WIFI_SSID
#define PIO_WIFI_SSID ""
#endif

#ifndef PIO_WIFI_PASSWORD
#define PIO_WIFI_PASSWORD ""
#endif

namespace config {
	// Wi-Fi credentials (from platformio_secrets.ini) and network names
	const char WIFI_SSID[] = PIO_WIFI_SSID;
	const char WIFI_PASSWORD[] = PIO_WIFI_PASSWORD;
	const char HOSTNAME[] = "christmas-matrix"; // http://christmas-matrix.local, and the name for OTA updates

	// Clock (Central European Time, with daylight saving time)
	const char TIMEZONE[] = "CET-1CEST,M3.5.0,M10.5.0/3";
	const char NTP_SERVER[] = "pool.ntp.org";
	const char NTP_BACKUP_SERVER[] = "time.google.com";

	// Pin assignments
	const uint8_t DATA_PIN = 13; // Data line of the LED matrix, on its left edge

	// Animation timing
	const uint32_t FRAME_MS = 33; // About 30 frames per second (a full frame takes 26 ms to send)
	const float SCENE_SECONDS = 20.0f; // Time each scene shows when they rotate
	const float FADE_SECONDS = 1.0f; // Crossfade between two scenes

	// LED current estimate (WS2812B): about 20 mA per color channel at full drive, and 1 mA per LED at rest
	const uint32_t MILLIAMPS_PER_CHANNEL = 20;
	const uint32_t IDLE_MILLIAMPS_PER_LED = 1;

	// Default settings and their limits
	const bool DEFAULT_ON = true;
	const uint8_t DEFAULT_BRIGHTNESS = 60;
	const int DEFAULT_SCENE = -1; // -1 rotates through all the scenes
	const uint32_t DEFAULT_MAX_MILLIAMPS = 3000;
	const bool DEFAULT_COUNTDOWN = true;
	const char DEFAULT_MESSAGE[] = "BUON NATALE * MERRY CHRISTMAS";
	const uint32_t MIN_MAX_MILLIAMPS = 1000;
	const uint32_t MAX_MAX_MILLIAMPS = 60000;
	const size_t MAX_MESSAGE_LENGTH = 200;
}