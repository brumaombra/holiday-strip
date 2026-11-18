#include "server.h"

#include <gizmo/device.h>

#include "config/config.h"
#include "display/display.h"
#include "player/player.h"
#include "scenes/scenes.h"
#include "settings/settings.h"
#include "settings/settings_json.h"

// Web server shared by the web app, its API and the Wi-Fi setup
AsyncWebServer server(80);

// Build the state the web app shows: the settings, the scenes and what the matrix is doing
JsonDocument buildStateData() {
	JsonDocument doc;
	JsonObject data = doc.to<JsonObject>();

	// Add the settings
	writeSettingsToJson(data, getSettings());

	// Add the holidays and their scenes, in the order they rotate
	JsonArray holidays = data["holidays"].to<JsonArray>();
	for (int i = 0; i < HOLIDAY_COUNT; i++) {
		const Holiday &holiday = getHoliday(i);
		JsonObject holidayJson = holidays.add<JsonObject>();
		holidayJson["id"] = holiday.id;
		holidayJson["name"] = holiday.name;
		JsonArray scenes = holidayJson["scenes"].to<JsonArray>();
		for (int j = 0; j < holiday.sceneCount; j++) {
			JsonObject sceneJson = scenes.add<JsonObject>();
			sceneJson["id"] = holiday.scenes[j]->id();
			sceneJson["name"] = holiday.scenes[j]->name();
		}
	}

	// Add what the matrix is doing (written by the main loop, read here as whole words)
	data["activeHoliday"] = getCurrentHolidayIndex();
	data["currentScene"] = getCurrentSceneIndex();
	data["estimatedMilliamps"] = getEstimatedMilliamps();

	// Add the limits the web app checks before sending
	data["minMaxMilliamps"] = config::MIN_MAX_MILLIAMPS;
	data["maxMaxMilliamps"] = config::MAX_MAX_MILLIAMPS;
	data["maxMessageLength"] = config::MAX_MESSAGE_LENGTH;

	// Return the built JSON document
	return doc;
}

// Register the routes and start the server (after the Wi-Fi setup registered its own)
void setupServer() {
	gizmo::setupDeviceApi(server, { config::PROJECT_NAME, config::FIRMWARE_VERSION });
	setupStateGetRoute();
	setupSettingsPostRoute();
	setupFrameGetRoute();
	setupWebAppRoute();
	server.begin();
}