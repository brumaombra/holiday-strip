#include "../server.h"

#include <gizmo/log.h>

#include "scenes/scenes.h"
#include "settings/settings.h"
#include "settings/settings_json.h"

namespace {
	// Change the settings from the JSON body (the main loop applies them on its next frame)
	void handleSettingsBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
		// Read the JSON body (gizmo-common checks CORS and the body size)
		JsonDocument doc;
		if (!gizmo::readJsonBody(request, data, len, index, total, doc)) {
			return;
		}

		// Check and apply the fields to a copy of the settings
		Settings settings = getSettings();
		const char *invalidField = applySettingsFromJson(doc.as<JsonObjectConst>(), settings, getSceneCount());
		if (invalidField != nullptr) {
			gizmo::printLogfln("Invalid settings field: %s", invalidField);
			sendErrorResponse(request, 400, "invalid_settings_field", invalidField);
			return;
		}

		// Save them and make them current
		if (!updateSettings(settings)) {
			sendErrorResponse(request, 500, "settings_save_failed", "Failed to save settings");
			return;
		}

		// Answer with the new state
		JsonDocument state = buildStateData();
		sendSuccessResponse(request, 200, &state);
	}
}

// Change the settings
void setupSettingsPostRoute() {
	// POST to change any of the settings (the answer is sent by the body handler)
	server.on("/api/settings", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, handleSettingsBody);

	// OPTIONS request for preflighted requests
	server.on("/api/settings", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
		sendCorsPreflightResponse(request, "POST, OPTIONS");
	});
}