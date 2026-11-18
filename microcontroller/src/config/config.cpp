#include "config.h"

#ifndef PIO_FIRMWARE_VERSION
#define PIO_FIRMWARE_VERSION "dev"
#endif

namespace config {
	// Project details shown in the web app (the release build sets the version)
	const char PROJECT_NAME[] = "Holiday Strip";
	const char FIRMWARE_VERSION[] = PIO_FIRMWARE_VERSION;

	// Network names and the Wi-Fi setup
	const char HOSTNAME[] = "holiday-strip"; // http://holiday-strip.local
	const char ACCESS_POINT_NAME[] = "Holiday-Strip"; // Open hotspot for the Wi-Fi setup, when no saved network works
	const char SETUP_PAGE_PATH[] = "/ap-setup"; // Page of the web app the hotspot clients are sent to
	const char ASSET_PATH_PREFIX[] = "/_nuxt/"; // Files of the web app, never redirected by the hotspot

	// Clock (Central European Time, with daylight saving time)
	const char TIMEZONE[] = "CET-1CEST,M3.5.0,M10.5.0/3";
	const char NTP_SERVER[] = "pool.ntp.org";
	const char NTP_BACKUP_SERVER[] = "time.google.com";

	// Pin assignments
	const uint8_t DATA_PIN = 16; // Data line of the LED matrix, on its left edge: ESP32 DevKit pin 16 (RX2 on most boards), no boot-strapping role

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
	const int DEFAULT_HOLIDAY = -1; // -1 follows the calendar
	const int DEFAULT_SCENE = -1; // -1 rotates through all the scenes of the holiday
	const uint32_t DEFAULT_MAX_MILLIAMPS = 3000;
	const bool DEFAULT_COUNTDOWN = true;
	const uint8_t DEFAULT_MESSAGE_COLOR = 0; // White
	const uint32_t MIN_MAX_MILLIAMPS = 1000;
	const uint32_t MAX_MAX_MILLIAMPS = 8000; // Highest limit the web app can set: keep it at about 80% of what the 5 V power supply delivers
}