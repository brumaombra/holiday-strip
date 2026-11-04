#include "web_server.h"

#include <WebServer.h>

#include "config/config.h"
#include "display/display.h"
#include "player/player.h"
#include "scenes/scenes.h"
#include "settings/settings_rules.h"
#include "web/index_html.h"

namespace {
	WebServer server(80);
	Settings *currentSettings = nullptr;

	// Escape a text for a JSON string
	String jsonEscape(const String &text) {
		String escaped;
		for (char c : text) {
			if (c == '"' || c == '\\') {
				escaped += '\\';
			}
			escaped += c;
		}
		return escaped;
	}

	// GET /api/state: the settings, the scene on the matrix, the estimated current and the scene names
	void handleState() {
		const Settings &settings = *currentSettings;
		String json = "{\"on\":" + String(settings.on ? "true" : "false");
		json += ",\"brightness\":" + String(settings.brightness);
		json += ",\"scene\":" + String(settings.scene);
		json += ",\"current_scene\":" + String(getCurrentSceneIndex());
		json += ",\"max_milliamps\":" + String(settings.maxMilliamps);
		json += ",\"estimated_milliamps\":" + String(getEstimatedMilliamps());
		json += ",\"countdown\":" + String(settings.countdown ? "true" : "false");
		json += ",\"message\":\"" + jsonEscape(settings.message) + "\"";
		json += ",\"scenes\":[";

		// Scene names, in the order they rotate
		for (int i = 0; i < getSceneCount(); i++) {
			if (i) {
				json += ",";
			}
			json += "\"" + String(getScene(i)->name()) + "\"";
		}
		json += "],\"uptime_s\":" + String(millis() / 1000) + "}";
		server.send(200, "application/json", json);
	}

	// GET /api/set?on=1&brightness=80&scene=-1&message=...&countdown=1&max_milliamps=3000: change the settings that are given, save them, and answer with the state
	void handleSet() {
		Settings &settings = *currentSettings;
		if (server.hasArg("on")) {
			settings.on = server.arg("on") == "1";
		}
		if (server.hasArg("brightness")) {
			settings.brightness = sanitizeBrightness(server.arg("brightness").toInt());
		}
		if (server.hasArg("scene")) {
			settings.scene = sanitizeScene(server.arg("scene").toInt(), getSceneCount());
		}
		if (server.hasArg("max_milliamps")) {
			settings.maxMilliamps = sanitizeMaxMilliamps(server.arg("max_milliamps").toInt());
		}
		if (server.hasArg("countdown")) {
			settings.countdown = server.arg("countdown") == "1";
		}

		// Message in capitals, cut to the longest message
		if (server.hasArg("message")) {
			char message[config::MAX_MESSAGE_LENGTH + 1];
			sanitizeMessage(server.arg("message").c_str(), message, sizeof(message));
			settings.message = message;
		}

		// Apply, save and answer
		setGreetingMessage(settings.message);
		setCountdownEnabled(settings.countdown);
		saveSettings(settings);
		handleState();
	}

	// GET /api/frame: raw RGB of the last frame (72 x 12, row by row), before gamma and brightness
	void handleFrame() {
		server.send_P(200, "application/octet-stream", (const char *)getOutputFrame(), sizeof(Frame));
	}
}

// Start the web server: the control page and the API
void setupWebServer(Settings &settings) {
	currentSettings = &settings;
	server.on("/", [] {
		server.send_P(200, "text/html", INDEX_HTML);
	});
	server.on("/api/state", handleState);
	server.on("/api/set", handleSet);
	server.on("/api/frame", handleFrame);
	server.begin();
}

// Answer the pending requests
void handleWebClients() {
	server.handleClient();
}