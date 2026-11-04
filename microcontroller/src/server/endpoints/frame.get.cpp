#include "../server.h"

#include "player/player.h"

// Get the frame on the matrix, for the live preview of the web app
void setupFrameGetRoute() {
	server.on("/api/frame", HTTP_GET, [](AsyncWebServerRequest *request) {
		// Validate CORS
		if (!ensureCorsAllowed(request)) {
			return;
		}

		// Send the raw RGB of the last frame (72 x 12, row by row, before gamma and brightness). It's read while the main loop draws the next one, so a preview frame can mix two frames, which is fine for a preview and costs no copy
		AsyncWebServerResponse *response = request->beginResponse(200, "application/octet-stream", (const uint8_t *)getOutputFrame(), sizeof(Frame));
		response->addHeader("Cache-Control", "no-store");
		request->send(response);
	});
}