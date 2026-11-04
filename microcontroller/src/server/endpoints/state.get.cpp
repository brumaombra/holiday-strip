#include "../server.h"

// Get the settings and what the matrix is doing
void setupStateGetRoute() {
	server.on("/api/state", HTTP_GET, [](AsyncWebServerRequest *request) {
		// Validate CORS
		if (!ensureCorsAllowed(request)) {
			return;
		}

		// Send the state
		JsonDocument doc = buildStateData();
		sendSuccessResponse(request, 200, &doc);
	});
}