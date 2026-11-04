#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

namespace config {
	// Network names and the Wi-Fi setup
	extern const char HOSTNAME[];
	extern const char ACCESS_POINT_NAME[];
	extern const char SETUP_PAGE_PATH[];
	extern const char ASSET_PATH_PREFIX[];

	// Wireless updates
	extern const char OTA_PASSWORD[];

	// Clock
	extern const char TIMEZONE[];
	extern const char NTP_SERVER[];
	extern const char NTP_BACKUP_SERVER[];

	// Pin assignments
	extern const uint8_t DATA_PIN;

	// Animation timing
	extern const uint32_t FRAME_MS;
	extern const float SCENE_SECONDS;
	extern const float FADE_SECONDS;

	// LED current estimate (WS2812B)
	extern const uint32_t MILLIAMPS_PER_CHANNEL;
	extern const uint32_t IDLE_MILLIAMPS_PER_LED;

	// Default settings and their limits
	extern const bool DEFAULT_ON;
	extern const uint8_t DEFAULT_BRIGHTNESS;
	extern const int DEFAULT_SCENE;
	extern const uint32_t DEFAULT_MAX_MILLIAMPS;
	extern const bool DEFAULT_COUNTDOWN;
	extern const char DEFAULT_MESSAGE[];
	extern const uint32_t MIN_MAX_MILLIAMPS;
	extern const uint32_t MAX_MAX_MILLIAMPS;
	constexpr size_t MAX_MESSAGE_LENGTH = 200; // In the header, because it sizes the message buffers
}

#endif